#include "CubeService.h"
#include "modules-resources/resources.h"
#include "common/core/debug.h"
#include "common/core/dbgassert.h"
#include "meas_acc/resources.h"
#include "whiteboard/builtinTypes/UnknownStructure.h"

#include <float.h>
#include <math.h>


const char* const CubeService::LAUNCHABLE_NAME = "Cube";

static const whiteboard::ExecutionContextId sExecutionContextId =
    WB_RES::LOCAL::GC_CUBE_POSITION::EXECUTION_CONTEXT;

static const whiteboard::LocalResourceId sProviderResources[] = {
    WB_RES::LOCAL::GC_CUBE_POSITION::LID,
};

CubeService::CubeService()
    : ResourceClient(WBDEBUG_NAME(__FUNCTION__), sExecutionContextId),
      ResourceProvider(WBDEBUG_NAME(__FUNCTION__), sExecutionContextId),
      LaunchableModule(LAUNCHABLE_NAME, sExecutionContextId),
      isRunning(false)
{
    position = 0;
    old_pos = 0;
    oldX = 0.0;
    oldY = 0.0;
    oldZ = 0.0;
}

CubeService::~CubeService()
{
}

bool CubeService::initModule()
{
    if (registerProviderResources(sProviderResources) != whiteboard::HTTP_CODE_OK)
    {
        return false;
    }

    mModuleState = WB_RES::ModuleStateValues::INITIALIZED;
    return true;
}

void CubeService::deinitModule()
{
    unregisterProviderResources(sProviderResources);
    mModuleState = WB_RES::ModuleStateValues::UNINITIALIZED;
}

/** @see whiteboard::ILaunchableModule::startModule */
bool CubeService::startModule()
{
    mModuleState = WB_RES::ModuleStateValues::STARTED;
    return true;
}

void CubeService::onGetRequest(const whiteboard::Request& request,
                                      const whiteboard::ParameterList& parameters)
{
    DEBUGLOG("CubeService::onGetRequest() called.");

    if (mModuleState != WB_RES::ModuleStateValues::STARTED)
    {
        return returnResult(request, wb::HTTP_CODE_SERVICE_UNAVAILABLE);
    }

    switch (request.getResourceConstId())
    {
    case WB_RES::LOCAL::GC_CUBE_POSITION::ID:
    {
        return returnResult(request, whiteboard::HTTP_CODE_OK,
                            ResponseOptions::Empty, position);
    }

    break;

    default:
        // Return error
        return returnResult(request, whiteboard::HTTP_CODE_NOT_IMPLEMENTED);
    }
}

void CubeService::onUnsubscribeResult(whiteboard::RequestId requestId,
                                             whiteboard::ResourceId resourceId,
                                             whiteboard::Result resultCode,
                                             const whiteboard::Value& rResultData)
{
    DEBUGLOG("CubeService::onUnsubscribeResult() called.");
}

void CubeService::onSubscribeResult(whiteboard::RequestId requestId,
                                           whiteboard::ResourceId resourceId,
                                           whiteboard::Result resultCode,
                                           const whiteboard::Value& rResultData)
{
    DEBUGLOG("CubeService::onSubscribeResult() called. resourceId: %u, result: %d", resourceId.localResourceId, (uint32_t)resultCode);

    whiteboard::Request relatedIncomingRequest;
    bool relatedRequestFound = mOngoingRequests.get(requestId, relatedIncomingRequest);

    if (relatedRequestFound)
    {
        returnResult(relatedIncomingRequest, wb::HTTP_CODE_OK);
    }
}

whiteboard::Result CubeService::startRunning(whiteboard::RequestId& remoteRequestId)
{
    DEBUGLOG("CubeService::startRunning()");

    // Reset 0G detection count
    mSamplesSince0GStart = 0;

    // Subscribe to LinearAcceleration resource (updates at ACC_SAMPLERATE Hz), when subscribe is done, we get callback
    wb::Result result = getResource("Meas/Acc/13", mMeasAccResourceId);
    if (!wb::RETURN_OKC(result))
    {
        return whiteboard::HTTP_CODE_BAD_REQUEST;
    }
    result = asyncSubscribe(mMeasAccResourceId, AsyncRequestOptions(&remoteRequestId, 0, true));

    if (!wb::RETURN_OKC(result))
    {
        DEBUGLOG("asyncSubscribe threw error: %u", result);
        return whiteboard::HTTP_CODE_BAD_REQUEST;
    }
    isRunning = true;
    return whiteboard::HTTP_CODE_ACCEPTED;
}

whiteboard::Result CubeService::stopRunning()
{
    if (!isRunning) {
        return whiteboard::HTTP_CODE_ACCEPTED;
    }

    DEBUGLOG("CubeService::stopRunning()");
    if (isResourceSubscribed(WB_RES::LOCAL::GC_CUBE_POSITION::ID) == wb::HTTP_CODE_OK )
    {
        DEBUGLOG("CubeService::stopRunning() skipping. Subscribers still exist.");
        return whiteboard::HTTP_CODE_ACCEPTED;
    }

    // Unsubscribe the LinearAcceleration resource, when unsubscribe is done, we get callback
    wb::Result result = asyncUnsubscribe(mMeasAccResourceId, NULL);
    if (!wb::RETURN_OKC(result))
    {
        DEBUGLOG("asyncUnsubscribe threw error: %u", result);
    }
    isRunning = false;
    releaseResource(mMeasAccResourceId);

    return whiteboard::HTTP_CODE_ACCEPTED;
}

// This callback is called when the acceleration resource notifies us of new data
void CubeService::onNotify(whiteboard::ResourceId resourceId, const whiteboard::Value& value,
                                  const whiteboard::ParameterList& parameters)
{
    // Confirm that it is the correct resource
    switch (resourceId.localResourceId)
    {
    case WB_RES::LOCAL::MEAS_ACC_SAMPLERATE::LID:
    {
        const WB_RES::AccData& linearAccelerationValue =
            value.convertTo<const WB_RES::AccData&>();

        if (linearAccelerationValue.arrayAcc.size() <= 0)
        {
            // No value, do nothing...
            return;
        }

        const whiteboard::Array<whiteboard::FloatVector3D>& arrayData = linearAccelerationValue.arrayAcc;

        for (size_t i = 0; i < arrayData.size(); i++)
        {
            whiteboard::FloatVector3D accValue = arrayData[i];
	    float dX = accValue.mX - oldX;
	    float dY = accValue.mY - oldY;
	    float dZ = accValue.mZ - oldZ;
	    oldX = accValue.mX;
	    oldY = accValue.mY;
	    oldZ = accValue.mZ;
	    if( (-0.3 < dX && dX < 0.3) && (-0.3 < dY && dY < 0.3 ) && 
	        (-0.3 < dZ && dZ < 0.3 ) )
	    {
		if( oldZ > 3.5 )
		{
		    if( oldX < -4.0 )
			position = 3;
		    else if( oldX > 4.0 )
			position = 4;
		    else
		    {
			if( oldY < -4.0 )
			    position = 8;
			else if( oldY > 4.0 )
			    position = 7;
			else
			   position = 0;
		    }
		}
		else if( oldZ < -3.5 )
		{
		    if( oldX < -4.0 )
			position = 5;
		    else if( oldX > 4.0 )
			position = 6;
		    else
		    {
			if( oldY < -4.0 )
			    position = 2;
			else if( oldY > 4.0 )
			    position = 1;
			else
			   position = 0;
		    }
		}
		if( old_pos != position )
		{
    		    updateResource( WB_RES::LOCAL::GC_CUBE_POSITION(),
                    		    ResponseOptions::Empty, position );
		    old_pos = position;
		}
	    }
        }
    }
    break;
    }
}

void CubeService::onSubscribe(const whiteboard::Request& request,
                                     const whiteboard::ParameterList& parameters)
{
    DEBUGLOG("CubeService::onSubscribe()");

    switch (request.getResourceConstId())
    {
    case WB_RES::LOCAL::GC_CUBE_POSITION::ID:
    {
        // Someone subscribed to our service. If no subscribers before, start running
        if (!isRunning)
        {
            whiteboard::RequestId remoteRequestId;
            whiteboard::Result result = startRunning(remoteRequestId);

            if (result == whiteboard::HTTP_CODE_ACCEPTED)
            {
                bool queueResult = mOngoingRequests.put(remoteRequestId, request);
                (void)queueResult;
                WB_ASSERT(queueResult);
            }
            else
            {
                return returnResult(request, whiteboard::HTTP_CODE_BAD_REQUEST);
            }

        }
        else{
            return returnResult(request, whiteboard::HTTP_CODE_OK);
        }

        break;
    }
    default:
        ASSERT(0); // Should not happen
    }
}

void CubeService::onUnsubscribe(const whiteboard::Request& request,
                                       const whiteboard::ParameterList& parameters)
{
    DEBUGLOG("CubeService::onUnsubscribe()");

    switch (request.getResourceConstId())
    {
    case WB_RES::LOCAL::GC_CUBE_POSITION::ID:
        returnResult(request, stopRunning());
        break;

    default:
        returnResult(request, wb::HTTP_CODE_BAD_REQUEST);
        break;
    }
}

void CubeService::onRemoteWhiteboardDisconnected(whiteboard::WhiteboardId whiteboardId)
{
    DEBUGLOG("CubeService::onRemoteWhiteboardDisconnected()");
    stopRunning();
}

void CubeService::onClientUnavailable(whiteboard::ClientId clientId)
{
    DEBUGLOG("CubeService::onClientUnavailable()");
    stopRunning();
}

# movesense-cube-module
Detect octahedrons upmost side. Movesense Launchable module

## Being translated

This is simple demo of howto make a Movesense launchable module. Also 
demonstrates benefits of processing measurements on sensor and notifying
phone app only when necessary. In this case when octahedron has a new stable position.

## Android app

Cube.apk can be downloaded from http://cloud.greencurrent.com/Cube.apk

It's rather large because it was made with ReactNative. Hopefully that means that
we'll have a iOS version in near future.

## Octahedron

Idea is that you program a movesense sensor with this module and place it inside
octahedron. You can make your own octahedron from http://cloud.greencurrent.com/Cube.pdf

## Future

You can connect Cube to MQTT-server and configure different message for each side and
control "something" by just turning cube to correct side.

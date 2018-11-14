# movesense-cube-module
Detect octahedrons upmost side. Movesense Launchable module

## Being translated

This is simple demo of howto make a Movesense launchable module. Also 
demonstrates benefits of processing measurements on sensor and notifying
phone app only when necessary. In this case when octahedron has a new stable position.

## Android app

Cube.apk can be downloaded from http://cloud.greencurrent.com/Cube.apk

It's rather large
Kyse on aika simppelistä, ehkä enemmän demosta, toteutuksesta jota 
tarkoituksenani on laajentaa eteenpäin.
Mutta saattaisi jo sellaisenaan olla hauska messudemo esim...

Eli moduuli itsessään tunnistaan yhden kahdeksasta asennosta. ( Tulevaisuudessa ehkä kuudesta, eli arpakuutiona!! ). Tein tulostettavan PDF-tiedoston josta voi askarrella 8-kulmion jonka keskelle Movesense-sensori kiinnitetään. Kuutiosta tulee jo ihan toimiva perus laser paperilla. Tarra-arkille tulostaminen helpottaa jo jäykkyytensä takia kasaamista, ja liimaelementit tulee automaattisesti mukana. Läpinäkyvä kalvo saattaisi olla myös kätevä, silloin sensori näkyisi hyvin sisältä.

Lisukkeena on Android Appi joka osaa avata moduulin tarjoaman palvelun. Eli ilman isompia konffauksia näyttää ylöspäin olevan sivun numeron.

Tarkoituksenani on lisätä mahdollisuus konffata joka sivuun lähetettävä MQTT-viesti. Tällöin 8-kulmiota voisi käyttää esim. kotiautomaatiojärjestelmän ohjauksessa. Asettaa esim. kotona/poissa tilan tai sytyttää/sammuttaa halutun valon. Vain mielikuvitus on rajana.

Tosin tuossa käytössä isoin este on MSD-kirjaston puuttuminen Linux-ympäristöstä. Android puhelimella voi vain demota ei tehdä pysyvämpää asennusta johtuen Androidista.


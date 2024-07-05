# RFID-master-card-door-lock-system

This system has one registered RFID card as the master card of the system. So, the master
card can be used to register other RFID cards/tags to the system to control the door lock.

Initially, the Master RFID card of the system is needed to define. Then, it can be used to
add an RFID Card/tag into the system to access the door lock. Similarly, Master Card can
be removed from the access of RFID card/tag as well. (Note: In the same way you can add
multiple cards/tags and remove them.) When scanning the wrong tag, the door is remaining
close, and the buzzer is on.

The LCD show the message when the system accepting the tag and opening the
door, access denied, successfully registered the tag with the tag’s UID, removed the tag’s
UID, and waiting for access.

The LEDs indicate whether the system accepts the tag and opens the door or whether an
entry is refused. The system also contains a reset button that may erase all data from the
system and return it to its initial state.

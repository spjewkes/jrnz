# Project JRNZ

This project aims to emulate a Z80 chip with the specific aim of running the ZX Spectrum 48k ROM. Therefore it will be heavily tailored towards the architecture of that machine.

Please note that development is very much work-in-progress. The principle is to develop this in a fairly organic way as part of its aim is to be simply a hobby project (i.e. something to tinker with in my spare time). This project is in no way a complete/accurate/good emulator and I would not recommend it for any serious emulation. There are plenty of better ones out there!

# Milestones

* 25/05/2017 - enough instructions implemented to run start of ZX Spectrum ROM (without interruption handling)
* 04/09/2019 - added interrupt handling (mode 1 only for the time being)
* 08/09/2019 - added rudimentary keyboard input and a display (albeit with no colour)
* 03/10/2019 - first game is playable. A Manic Miner snapshot can now be loaded and played. There is no colour or sound and the game crashes once it is game over.
* 04/10/2019 - colour has now been added and the Manic Miner snapshot is playable without crashes.
* 20/10/2019 - bug fixed to allow simple basic "10 print "message"; 20 goto 10" to run
* 31/03/2024 - fixed bug with NEG instruction so that Jet Set Willy is no longer broken when you enter the second room
* 05/04/2024 - Z80 instruction exerciser (documented instructions only) now runs to completion without missing instructions. Although some instructions are still failing their checksums.
* 06/04/2024 - Z80 instruction exerciser (documented instructions only) now passes all tests.
* 10/04/2024 - Implemented support for Z80 file format (version 1 only)
* 15/04/2024 - Z80 file format now supports version 2 and 3 (48k mode only)

# cyclone-xTreme
The original UCI chess engine from 2009 

![alt tag](https://raw.githubusercontent.com/FireFather/cyclone-xTreme/master/src/cyclone_banner.jpg)


- based on:
Fruit 2.1 by Fabien Letouzey, Toga II by Thomas Gaksch, Grapefruit 1.0 by Vadim Demichev, & Toga CMLX 145e4 by Teemu Pudas


## features:
- Cyclone xTreme plays normal chess as well as Fischer Random Chess (Chess 960).
To play Fischer Random, enable UCI option UCI_Chess960.

- can use a configuration file (Cyclone.cfg) to obtain more than 140 eval, material, and search parameters.

- The program will write a default configuration file...just type 'default' at the command prompt....the chess engine should play strong utilizing the default configuration.

- Or type 'random' at the command prompt.  A new Cyclone.cfg will be written using random values
(these values are reasonable and within a set range, yet are relatively broad in scope).

- To widen or narrow the random range of each parameter requires editing the
source code.

- WARNING: any existing Cyclone.cfg file that may exist in the current directory will be overwritten.
- WARNING: using a random Cyclone.cfg file will likely weaken the engine significantly.

- The Cyclone.cfg file can also be manually edited for tuning...using any (plain text)
word processor...Wordpad, Notepad, vi, etc.
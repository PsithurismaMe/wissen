# wissen
A utility to help users learn German

Verbs are assumed to be conjugated to present tense.

Compilation Instructions:
-----------------------------------------------
Dependencies: <br/>
	+ A C++ compiler <br/>
	+ NCURSES with support for wide characters <br/> 
	+ Fonts which can render the following correctly: ä, ö, ü, ß <br/>

To install all dependencies in one command:
(These assume you already have the correct fonts)

  ```
Arch Based:
pacman -S ncurses gcc
  ```
  ```
Debian Based:
apt install libncurses-dev g++
  ```
<br/>
If you want to compile this on Windows, use WSL or a linux VM unless you want to go through dependency hell.
	

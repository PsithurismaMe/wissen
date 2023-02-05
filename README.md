# Wissen
## A utility to help users learn German
+ Features
  + Multiple choice word translation
  + Verb conjugation grading

## Compilation Instructions:
-----------------------------------------------
### Dependencies
+ A C++ compiler
+ NCURSES with support for wide characters
+ Fonts which can render the following correctly: ä, ö, ü, ß

### To install all dependencies in one command:
### Arch Based Linux Distros:
  ```
pacman -S ncurses gcc
  ```
### Debian Based Linux Distros:
  ```
apt install libncurses-dev g++
  ```
### To actually compile it
  ```
  g++ ui.cpp -lncursesw -O3
  ```

If you want to compile this on Windows, use WSL or a Linux VM unless you want to go through dependency hell.
	
### Legal
All files in this repository are licensed under the [GPL-V3](LICENSE).
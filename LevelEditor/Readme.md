# Level Editor

### SETUP
  - [Windows](#setup---windows)
  - [Ubuntu](#setup---ubuntu)
### Setup - Windows
`Edit the following lines to link to your SDL files. _i for include, _l for lib`
```
SDLMAIN_i = 
SDLMAIN_l = 
SDLIMG_i = 
SDLIMG_l =
```
Ex:
```
SDLMAIN_i = -I"PathToSDL\include\SDL2"
SDLMAIN_i = =IC:\include\SDL2
```
### Setup - Ubuntu
`sudo apt install build-essential g++ libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev -y`
## Images

<img src="https://github.com/RicardoJarrett/cpp_stuff/blob/main/LevelEditor/Sample.png" width="400">
<img src="https://github.com/RicardoJarrett/cpp_stuff/blob/main/LevelEditor/test.png" width="200">

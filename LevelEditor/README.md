Edit the following lines to link to your SDL files.

SDLMAIN_i = 
SDLMAIN_l = 
SDLIMG_i = 
SDLIMG_l =

Make sure to include the \SDL2 directory.
Both _i for include and _l for lib
Ex:

SDLMAIN_i = -I"PathToSDL\include\SDL2"
SDLMAIN_i = =IC:\include\SDL2
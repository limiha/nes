# NES Emulator Implementation

## Build and Run
1. Open solution in Visual Studio (VS 2015 is the only thing I've used so far).
2. Build Release configuration. This is very important, Debug only does about 7 frames per second.
3. run `nes.exe < path to .nes file >`

## Controls
```
Joypad 1
A           ->  Left Alt or S
B           ->  Left Ctrl or A
Select      ->  Right Shift or Backslash
Start       ->  Enter (Return)
Up          ->  Up
Down        ->  Down
Left        ->  Left
Right       ->  Right

Save State  -> F1
Load State  -> F2

Quit        -> Esc
```

## Acknowledgements
 - This was originally based on https://github.com/pcwalton/sprocketnes which was based on FCEU (http://www.fceux.com/web/home.html).
 - Nintendulator (http://www.qmtpro.com/~nes/nintendulator/) has been incredibly helpful both because of it's source and the debug tools built into it's emulator
 - NesDev (http://nesdev.com) and the NesDev Wiki (http://wiki.nesdev.com/w/index.php/Nesdev_Wiki)
 - SDL (https://www.libsdl.org/license.php)
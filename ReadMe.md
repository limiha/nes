NES Emulator implementation

#Build and Run
1. Open solution in Visual Studio (VS 2014 is the only thing I've used so far).
2. Build Release configuration. This is very important, Debug only does about 7 frames per second.
3. run `nes.exe < path to .nes file >

# Controls
```
Nes     ->  Keyboard
A       ->  Z
B       ->  X
Select  ->  Right Shift
Start   ->  Enter (Return)
Up      ->  Up
Down    ->  Down
Left    ->  Left
Right   ->  Right
```

# Acknowledgements
 - This is heavily borrowed from https://github.com/pcwalton/sprocketnes which was in turn based on FCEU (http://www.fceux.com/web/home.html). I have tried to write an emulator several times in the past, the most recent of which is https://github.com/chuckries/rustnes/
 - Nintendulator (http://www.qmtpro.com/~nes/nintendulator/) has been incredibly helpful both because of it's source and the debug tools built into it's emulator
 - NesDev (nesdev.com) and the NesDev Wiki (http://wiki.nesdev.com/w/index.php/Nesdev_Wiki)

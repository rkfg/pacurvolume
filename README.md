# pacurvolume

PulseAudio ncurses volume control written in C++

# Why?

There are [several](https://github.com/mk-fg/pulseaudio-mixer-cli) CLI [mixers](https://github.com/Valodim/pamixer)
already available so why make another one? Well, aside from the NIH-syndrome there's another reason, they're written in
Python. I wanted to use something compact and independent and also to code in modern C++11. That's why I created this.
The stripped binary size is only 60 Kib<sup>1</sup>, it has very few dependencies (C++ runtime, ncurses and libpulse),
runs pretty fast and should be easily portable to other platforms.

# Controls

Very few keys are used:

| Key    | Function                             |
|--------|--------------------------------------|
| Arrows | Select sink inputs and change volume |
| 1      | Reset sink input volume to 100%      |
| 2      | Set sink input volume to 50%         |
| 3      | Set sink input volume to 25%         |
| m or 0 | Mute/unmute sink input               |
| q      | Exit                                 |

# Features

Not so much but should be enough to feel comfortable.

* Terminal window resizing is supported, all controls are scaled
* Listens to PulseAudio events so any volume/mute changes made in another app are visible in realtime
* Supports scrolling if you have lots of them or if the terminal window is too small
* Supports Unicode, any non-ASCII sink input names should be displayed correctly
* Has proper mouse wheel support, although it's not documented in ncurses and may misbehave
* Shows sound activity per sink input with [+] symbol

# Dependencies

Requires libpulse-dev and libncursesw5-dev (note the "w", it's for Unicode support), also cmake is used as a
build system. If you don't care about Unicode or simply don't have the wide version of ncurses, delete or comment out
`set(CURSES_NEED_WIDE TRUE)` line in `CMakeLists.txt`, don't forget to remove `CMakeCache.txt` if you
tried to build it before!

# Bugs

The mixer doesn't handle PulseAudio-related errors like if the daemon isn't running or refuses connection.
This may result in segfaults or misbehavior. There also may be other errors when working with non-standart sink inputs
or multichannel inputs. I've tried to make it work as it should but I don't have a 5.1 audio system so YMMV.

On Android devices the interface looks broken when connecting via ConnectBot (SSH). It works fine but window boxes are 
torn apart. Could be a terminfo issue though Midnight Commander looks quite fine there.

## Footnotes

<sup>1</sup>if using cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS=-s

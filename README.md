# RadOS
Toy 16-bit real mode operating system

For now its more of a program running on the bare hardware rather than an actual operating system.
I originally started using assembly but it is a much slower process and I wasn't making much progress.
Using assembly may produce a smaller binary, especially in the startup code, but that hasn't been an issue as yet.

The bootloader is currently using [BootProg](https://github.com/alexfru/BootProg) as it can load a DOS com executable format.

I am using [Open Watcom V2](http://open-watcom.github.io/) 16-bit c compiler to produce the com file.

I am using some of the standard c functions, you just need to be careful which you use.
You can't use any of the standard file i/o, even standard output.
It is safe to use functions that don't rely on the OS such as the string functions.
I am currently using malloc/free. I wasn't sure it was going to work, but it seems to work ok.

Current focus is on screen output and keyboard input using the BIOS api.
Next I will look into loading a sector from the disk.
Eventually it will probably be worthwhile to replace BIOS with direct hardware access.


## Tools

[GNU Make](https://www.gnu.org/software/make/) v4.3

[Open Watcom V2](http://open-watcom.github.io/) 2.0 beta Sep 15 2020

[QEMU](https://qemu.weilnetz.de/) v5.1.0

[BootProg](https://github.com/alexfru/BootProg)

## Build
```
make build
```

## Run
```
make run
```

# Building with Visual Studio

Recently, I tried compiling the morphologica tests and examples on
Windows with Visual Studio 2019. There are quite a few changes that
are necessary to make the morphologica code Windows-compatible.
However, it's going to happen - I already got several morph::Visual
based program to run compiled natively with VS2019. Checkout the
branch ```compile_vs2019``` if you would like to help.

# Installation on Windows

While it is possible to compile and build morphologica on Windows (by
using Windows subsystem for Linux to provide a Linux environment) I
haven't had it fully working myself, and don't support it or suggest
that you try.

If you *really* want to try here are some hints, but you'll have to
solve the OpenGL-4.1-on-WSL problem that I wasn't able to.

## How to fail to fully install morphologica on Windows with Windows subsytem for Linux

To install on Windows, first install *Windows subsystem for Linux*
https://docs.microsoft.com/en-us/windows/wsl/install-win10

Install the Ubuntu 18.04 image from the Windows store. Fully upgrade
all packages before you start:

```sh
sudo apt-get install apt
sudo apt update
sudo apt upgrade
```

Now you can follow instructions for installing on GNU/Linux
(README.install.linux.md) to get some of the morphologica code to
compile.

In principle, if you install an X server on your Windows desktop, then
the graphical morphologica programs should be compilable. This is
required for some of the morphologica test programs to run. I tried
Xming, and I payed a donation to the developer to get the up-to-date
version of Xming (the free version will definitely not work for modern
OpenGL applications). In my Windows subsystem for Linux Ubuntu 18.04
environment I exported the DISPLAY environment variable so that
programs will know where to draw their output:

```sh
export DISPLAY=:0
```
(actually, I added this to my .bashrc).

However, Xming version 7.7 *didn't work for me* (morphologica requires
OpenGL version 4.1). I then tried VcXsrv, but that didn't work for me
either.

It *may* be possible to get one or other of these to work, but I gave up.

# Ubuntu Installation
## Windows Subsystem for Linux (WSL or WSL2)
Windows Subsystem for Linux is a compatibility layer for running Linux binary executables natively on Windows 10. This means you will be able to run Linux programs on your Windows machine as if you were running on a Linux machine. We found this process significantly easier and more reliable for the user who doesn't wish to dual-boot or use a virtual machine, and it's an alternative to Cygwin.

Follow the steps for installing the Linux subsystem [here](https://docs.microsoft.com/en-us/windows/wsl/install-win10)

Some additional tips and recommendations

- We recommend choosing Ubuntu as your Linux distro with version >= 16.04 as the rest of the guide will assume so.
## Ubuntu Installation
After finishing the steps, [follow the instructions for setting up GamesmanClassic for Ubuntu](/doc/build-instructions-linux.md), __BUT DO NOT DO__

```bash
./XGamesman.new # THIS SHOULD NOT WORK
```
Instead, proceed to the next step.

## X Server Installation
The previous steps allowed you to access GamesmanClassic under the Text User Interface (TUI). You should be able to run

```bash
cd bin
./mttt
```

As of the writing of this guide, WSL does not natively support Linux Graphical User Interfaces (GUIs)...[yet](https://www.theverge.com/2020/5/19/21263377/microsoft-windows-10-linux-gui-apps-gpu-acceleration-wsl-features). As a workaround, we will be using X Servers instead.

### Using Xming (WSL)
Download Xming from Sourceforge [here](https://sourceforge.net/projects/xming/), then execute the installer.

Some additional tips and recommendations

- One of the steps has an option to install an SSH client called PuTTY. This is purely optional and not required.
After installation, edit the `~/.bashrc` in your Linux distro by adding export DISPLAY=:0, then restarting your terminal. Run the Xming application and it should be running in the background.

Note: Whenever you run a GamesmanClassic X program (i.e. `./Xmttt3`), be sure to run the Xming X Server as well.

### WSL2
_Note: The following has been found not to work on some machines. We recommend that if you want to use an X Server with WSL2, try out different solutions on the internet._

Follow these [instructions](https://autoize.com/xfce4-desktop-environment-and-x-server-for-ubuntu-on-wsl-2/). No need to reinstall the distribution if you already have WSL2.

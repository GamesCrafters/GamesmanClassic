# Ubuntu Installation
## Windows Subsystem for Linux (WSL 2)
Windows Subsystem for Linux is a virtual machine platform that lets you install popular Linux distributions natively on Windows 10 or above. We found this process significantly easier and more reliable for the user who doesn't wish to dual-boot and it's an alternative to Cygwin.

- We recommend choosing Ubuntu as your Linux distro with version >= 16.04 as the rest of the guide will assume so.
- Installing WSL 2 requires enabling Windows Virtual Machine Platform, which is previously known to have conflicts with software including VirtualBox and Intel® Extreme Tuning Utility. One possible solution is to temporarily [turn off Windows Virtual Machine Platform and similar features](https://www.dell.com/support/kbdoc/en-ie/000195980/how-to-enable-or-disable-windows-virtualization-on-dell-systems) when you need access to applications that are in conflict with it.


## Ubuntu Installation
The recommended way of installing Ubuntu is through the Windows Store App. Simply search up "Ubuntu" and install the latest distribution. Alternatively, you can follow the steps for installing the Linux subsystem [here](https://learn.microsoft.com/en-us/windows/wsl/install) if you wish to install WSL using the terminal.

You will be prompted to set up your username and password for Ubuntu when you launch the virtual machine for the first time. __The username might need to be in all lowercase and contain no white spaces.__


After finishing the steps, [follow the instructions for setting up GamesmanClassic for Ubuntu](/doc/build-instructions-linux.md), __BUT CHECK THE BUILD VERSION OF YOUR WINDOWS OS BEFORE PROCEEDING WITH THE FOLLOWING STEP__

```bash
./XGamesman.new # THIS DOES WORK IN OLDER BUILDS OF WINDOWS 10.
```
If you have [Windows 10 Build 19044+ or Windows 11](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps), the above command should work and a new window should pop up. Otherwise, consider updating your OS or try the following workaround.

## X Server Installation
3/6/2023: This workaround solution has not been tested recently and maybe out of date. Please contact us if you run into any issue.

The previous steps allowed you to access GamesmanClassic under the Text User Interface (TUI). You should be able to run

```bash
cd bin
./mttt
```

### Using Xming (WSL)
Download Xming from Sourceforge [here](https://sourceforge.net/projects/xming/), then execute the installer.

Some additional tips and recommendations

- One of the steps has an option to install an SSH client called PuTTY. This is purely optional and not required.
After installation, edit the `~/.bashrc` in your Linux distro by adding export DISPLAY=:0, then restarting your terminal. Run the Xming application and it should be running in the background.

Note: Whenever you run a GamesmanClassic X program (i.e. `./Xmttt3`), be sure to run the Xming X Server as well.

### WSL2
_Note: The following has been found not to work on some machines. We recommend that if you want to use an X Server with WSL2, try out different solutions on the internet._

Follow these [instructions](https://autoize.com/xfce4-desktop-environment-and-x-server-for-ubuntu-on-wsl-2/). No need to reinstall the distribution if you already have WSL2.

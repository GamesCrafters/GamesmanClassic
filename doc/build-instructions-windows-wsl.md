# Build Instructions (Windows Subsystem for Linux 2)

Windows Subsystem for Linux is a virtual machine platform that lets you install popular Linux distributions natively on Windows 10 or above. This is the recommended way to build GamesmanClassic on Windows. See also [Build Instructions for Windows Using Cygwin](build-instructions-windows-cygwin.md).

- We recommend choosing Ubuntu as your Linux distro with version >= 20.04 as the rest of the guide will assume so.
- Installing WSL 2 requires enabling Windows Virtual Machine Platform, which was previously known to have conflicts with software including VirtualBox and Intel® Extreme Tuning Utility. One possible solution is to temporarily [disable Windows Virtual Machine Platform and similar features](https://www.dell.com/support/kbdoc/en-ie/000195980/how-to-enable-or-disable-windows-virtualization-on-dell-systems) when you need access to applications that are in conflict with them.

## WSL 2 Installation
The recommended way of installing Ubuntu is through the Windows Store App. Simply search up "Ubuntu" and install the latest distribution. Alternatively, you can follow the steps for installing the Linux subsystem [here](https://learn.microsoft.com/en-us/windows/wsl/install) if you wish to install WSL from the terminal.

If you are installing the latest version of Ubuntu, you might be prompted to update the WSL2 Kernel. Follow [Step 4 of this tutorial](https://learn.microsoft.com/en-us/windows/wsl/install-manual#step-4---download-the-linux-kernel-update-package).

You will be prompted to set up your username and password for Ubuntu when you launch the virtual machine for the first time. __The username might need to be in all lowercase letters and contain no white spaces.__


After finishing the steps, [follow the instructions for setting up GamesmanClassic on Linux](/doc/build-instructions-linux.md).

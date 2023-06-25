# Build Instructions (Linux)

## Prerequisites

- zlib
    - `zlib1g-dev`
- Build Tools
    - `build-essential`
    - `autoconf`

You may install these prerequisites with the following command:

```
sudo apt update && sudo apt upgrade

sudo apt install build-essential autoconf zlib1g-dev
```

### Optional Dependencies

The following are required to compile with the Tcl/Tk graphical user interface:

- Tcl/Tk
    - `tcl-dev`
    - `tk-dev`

```
sudo apt install tcl-dev tk-dev
```

Note: If you are running WSL on Windows, make sure your operating system meets the requirement for [Linux GUI apps on the Windows Subsystem for Linux](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps). As of June 25, 2023, the requirement is Windows 10 Build 19044+ or Windows 11.

## Building GamesmanClassic

Make sure you're in the directory of the repository. Then use the following script to build GamesmanClassic:

```bash
# Generate configure script
autoconf # Successful execution of this command should not generate any console output.

# Configure or reconfigure without graphics.
./configure --disable-graphics
```

Optional, if you successfully installed Tcl/Tk and would like to build with graphics, configure with the default settings instead:

```bash
# Configure or reconfigure with graphics.
./configure
```

If you want to change an option such as whether or not to build with graphics, then you may need to change the configuration options.
The configuration options are available here: [GamesmanClassic Configuration Options](build-configuration-options.md).

```bash
# Build GamesmanClassic
make
```

## Running GamesmanClassic

Once GamesmanClassic is configured and compiled, you can run it by `cd`ing to the `bin/` directory inside of the package and running a file that starts with an `m` (such as `machi`, or `mttt`), and play the game from the console.

```bash
# Change the working directory to "bin/"
cd bin

# Run the Tic-Tac-Toe text program for example
./mttt
```

To start up the optional graphical GamesmanClassic interface, you may go to the `bin/` directory and type:

```bash
# Run the new GUI
./XGamesman.new
```

Then you will be presented with a list of games currently in GamesmanClassic, and simply click to play.

Happy gaming!

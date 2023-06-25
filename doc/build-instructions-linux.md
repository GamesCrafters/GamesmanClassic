# Build Instructions (Ubuntu/Debian)

These instructions were tested on June 23, 2023 on the following operating systems:
- Ubuntu 22.04.2 LTS
- Windows Subsystem for Linux
    - Ubuntu 22.04.2 LTS
    - Debian GNU/Linux 11 (bullseye)

## Summary

You can complete required setup (installing and building GamesmanClassic without graphics) with this one command, assuming nothing goes wrong. Try pasting this in your terminal. The GamesmanClassic repo will be cloned to your current directory in the process.

```bash
sudo apt update && sudo apt upgrade && sudo apt install git build-essential autoconf zlib1g-dev && git clone https://github.com/GamesCrafters/GamesmanClassic.git && cd GamesmanClassic && autoconf && ./configure --disable-graphics && make && cd bin && ./mttt
```

If you are using one of the Berkeley EECS instructional computers, use the following command instead.

```bash
git clone https://github.com/GamesCrafters/GamesmanClassic.git && cd GamesmanClassic && autoconf && ./configure --disable-graphics && make && cd bin && ./mttt
```

Afterward, if you see a tic-tac-toe TextUI prompt, then you have completed required setup. If not, read the following more detailed instructions in order to troubleshoot. You should read them anyway to understand what just happened.

## Setup Instructions, in More Detail

### 1. Update Package Lists
Skip this step if you are using one of the Berkeley EECS instructional computers.
```bash
sudo apt update
```

### 2. Update Existing Packages
Skip this step if you are using one of the Berkeley EECS instructional computers.
```bash
sudo apt upgrade
```

### 3. Install Required Packages
The following packages are required to build GamesmanClassic:

- zlib
    - `zlib1g-dev`
- Build Tools
    - `build-essential`
    - `autoconf`

Skip this step if you are using one of the Berkeley EECS instructional computers; these packages should already be installed.

```bash
sudo apt install git build-essential autoconf zlib1g-dev
```

### 4. Clone GamesmanClassic
```bash
git clone https://github.com/GamesCrafters/GamesmanClassic.git

# Commands to build GamesmanClassic will be run from the base project directory.
cd GamesmanClassic
```

### 5. Configure and Compile GamesmanClassic
```bash
# Generate configure script
autoconf # Successful execution of this command should not generate any console output.

# Configure or reconfigure without graphics.
./configure --disable-graphics

# Compile
make
```

Note: If you ever change a Makefile.in file, you should rerun ./configure --disable-graphics then rerun make clean then make. If you just make a change to a C file, then you can recompile with make.

### 6. Run GamesmanClassic
Once GamesmanClassic is configured and compiled, you can run it by `cd`ing to the `bin/` directory inside of the package and running a file that starts with an `m` (such as `machi`, or `mttt`), and play the game from the console.

```bash
# Change the working directory to "bin/"
cd bin

# Run the Tic-Tac-Toe text program for example
./mttt
```

If this works, then you have successfully set up GamesmanClassic without graphics.

## (Optional) Build GamesmanClassic with Graphics

### Prerequisites
- Finish the required setup steps above.
- Not using Berkeley EECS instructional computers (including Hive) where Tcl/Tk are not supported.

Note: If you are running WSL on Windows, make sure your operating system meets the requirement for [Linux GUI apps on the Windows Subsystem for Linux](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps). As of June 25, 2023, the requirement is Windows 10 Build 19044+ or Windows 11.

### 1. Install Required Packages
Packages required to compile with the Tcl/Tk graphical user interface:

- Tcl/Tk
    - `tcl-dev`
    - `tk-dev`

```bash
sudo apt install tcl-dev tk-dev
```

### 2. Reconfigure and Recompile with Graphics
Make sure you are in the GamesmanClassic base project directory. Then run the following to recompile with graphics.

```bash
# Configure or reconfigure with graphics.
./configure

# Recompile
make clean && make
```

Explanations for configuration options are available here: [GamesmanClassic Configuration Options](build-configuration-options.md).

### 3. Running GamesmanClassic with Graphics

To start up the optional graphical GamesmanClassic interface,

```bash
cd bin

./XGamesman.new
```

Then you will be presented with a list of games currently in GamesmanClassic, and simply click to play.

Happy gaming!

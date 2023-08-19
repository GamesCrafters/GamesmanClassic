# Build Instructions (Ubuntu/Debian)

These instructions were tested on August 19, 2023 on Red Hat Enterprise Linux 9.2 with basic building tools installed at system installation.

## Summary

You can complete required setup (installing and building GamesmanClassic without graphics) with this one command, assuming nothing goes wrong. Try pasting this in your terminal. The GamesmanClassic repo will be cloned to your current directory in the process.

```bash
sudo dnf update && sudo dnf upgrade && sudo dnf install git gcc autoconf zlib && git clone https://github.com/GamesCrafters/GamesmanClassic.git && cd GamesmanClassic && autoconf && ./configure --disable-graphics && make && cd bin && ./mttt
```

Afterward, if you see a tic-tac-toe TextUI prompt, then you have completed required setup. If not, read the following more detailed instructions in order to troubleshoot. You should read them anyway to understand what just happened.

## Setup Instructions, in More Detail

### 1. Update Package Lists
```bash
sudo dnf update
```

### 2. Update Existing Packages
```bash
sudo dnf upgrade
```

### 3. Install Required Packages

Note on August 19, 2023: This was not tested without basic building tools installed at system installation. Additional packages might be needed.

```bash
sudo dnf install git gcc autoconf zlib
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

Note: If you ever change a `Makefile.in` file, you should rerun `./configure --disable-graphics` then rerun `make clean` then `make`. If you just make a change to a C file, then you can recompile with `make`.

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

### Prerequisite
- Finish the required setup steps above.

### 1. Install Required Packages
Packages required to compile with the Tcl/Tk graphical user interface:

- Tcl/Tk
    - `tcl-devel`
    - `tk-devel`

```bash
sudo dnf install tcl-devel tk-devel
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

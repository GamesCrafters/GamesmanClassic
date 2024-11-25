# Build Instructions (macOS)

Note: These instructions were tested on June 25, 2023 on macOS Ventura 13.0 and Tcl/Tk 8.6.13.

## Summary
You can complete required setup (installing and building GamesmanClassic without graphics) with this one command, assuming nothing goes wrong. Try pasting this in your terminal. The GamesmanClassic repo will be cloned to your current directory in the process.
```bash
brew install autoconf zlib && git clone https://github.com/GamesCrafters/GamesmanClassic.git && cd GamesmanClassic && autoconf && ./configure --disable-graphics && make && cd bin && ./mttt
```

Afterward, if you see a tic-tac-toe TextUI prompt, then you have completed required setup. If not, read the following more detailed instructions in order to troubleshoot. <span style="color:red">You should read them anyway to understand what just happened.</span>

## Setup Instructions, in More Detail

### 1. Install Git
If you already have Git installed, then continue to the next step; otherwise, follow these [Git installation instructions](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git).

### 2. Install Homebrew
If you already have [Homebrew](https://brew.sh/) installed, then continue to the next step; otherwise, run the following command to install it.
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. Install Required Packages
```bash
brew install autoconf zlib
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
autoconf

# Configure without graphics
./configure --disable-graphics

# Compile
make
```
Note: If you ever change a ```Makefile.in``` file, you should rerun ```./configure --disable-graphics``` then rerun ```make clean``` then ```make```. If you just make a change to a C file, then you can recompile with ```make```.

### 6. Run GamesmanClassic
Once GamesmanClassic is configured and compiled, you can run it by `cd`-ing to the `GamesmanClassic/bin/` directory and running a binary that starts with `m` (such as `machi`, or `mttt`), and play the game from the console.

```bash
# Must be in the "bin/" directory first
cd bin

# Run the Tic-Tac-Toe text program, for example
./mttt
```

If this works, then you have successfully set up GamesmanClassic without graphics.

## (Optional) Build GamesmanClassic with Graphics

Note: You must run the "Required Setup" steps before doing this.

First, install tcl-tk.
```bash
brew install tcl-tk
```

Run the following `configure` command from the GamesmanClassic base project directory (i.e., first `cd` out of `bin/`).

```
./configure --with-tcl=/opt/homebrew/opt/tcl-tk/lib/tclConfig.sh --with-tk=/opt/homebrew/opt/tcl-tk/lib/tkConfig.sh
```

If that command does not work (you may get a "Tcl not found" error), try this command instead:
```
./configure --with-tcl=/usr/local/opt/tcl-tk/lib/tclConfig.sh --with-tk=/usr/local/opt/tcl-tk/lib/tkConfig.sh
```

If that command does not work either, figure out where your Homebrew Tcl/Tk installation is and run `./configure --with-tcl=<path/to/tclConfig.sh> --with-tk=<path/to/tkConfig.sh>`.


After successful configuration can then compile and run the text programs as before.
```
make clean

make

cd bin

./mttt
```

If at any point you get a linker error, try running `make clean` and restarting from the `configure` command.

To start up the graphical GamesmanClassic interface, you may go to the `bin/` directory and type:

```bash
./XGamesman.new
```

Then you should be presented with a list of games currently in GamesmanClassic. Simply click to play (it is recommended to try tic-tac-toe first). If this works, then you have successfully set up GamesmanClassic with graphics.

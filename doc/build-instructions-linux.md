# Build Instructions (macOS)

## Prerequisites

- zlib
    - `zlib1g-dev`
- Tcl/Tk
    - `tcl-dev`
    - `tk-dev`
- Build Tools
    - `build-essential`
    - `autoconf`

You may install these prerequisites with the following command:

```
sudo apt install build-essential autoconf zlib1g-dev tcl-dev tk-dev
```

### Optional Dependencies

If you want to compile games with big integer support, you'll need the GMP library (latest version).

## Building GamesmanClassic

Make sure you're in the directory of the repository. Then use the following script to build GamesmanClassic.

```bash
# Generate configure script
autoconf

# Configure
./configure
```

If you want to change an option such as whether or not to build with graphics, then you may need to change the configuration options.
The configuration options are available here: [GamesmanClassic Configuration Options](build-configuration-options.md).

```bash
# Build GamesmanClassic
make
```

## Running GamesmanClassic

Once GamesmanClassic is configured and compiled, you can run it by cd'ing to the `bin/` directory inside of the package and running a file that starts with an `m` (such as `machi`, or `mttt`), and play the game from the console.

```bash
# Must be in the "bin/" directory first
cd bin

# Run the Tic-Tac-Toe text program for example
./mttt
```

To start up the graphical GamesmanClassic interface, you may go to the `bin/` directory and type:

```bash
# Run the new GUI
./XGamesman.new
```

Then you will be presented with a list of games currently in GamesmanClassic, and simply click to play.

Happy gaming!

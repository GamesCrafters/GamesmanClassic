# Build Instructions (macOS)

**The following building instructions have only been tested on macOS Catalina, but will likely work with older systems.**

## Prerequisites

- `autoconf`
- `zlib`
- `tcl-tk`

You can install the required packages with `brew`:

```bash
brew install autoconf zlib tcl-tk
```

### Optional Dependencies

If you want to compile games with big integer support, you'll need the GMP library (latest version).

After you're done editing & saving the init script, reopen a terminal session and the new init script should be in effect. (You can also use the `source` command to load the init script.)

## Building GamesmanClassic

Make sure you're in the directory of the repository. Then use the following script to build GamesmanClassic.

```bash
# Generate configure script
autoconf

# Configure with the installed tcl-tk
./configure --with-tcl=/opt/homebrew/opt/tcl-tk/lib/tclConfig.sh --with-tk=/opt/homebrew/opt/tcl-tk/lib/tkConfig.sh
```

If you get the following error `configure: error: cannot find required auxiliary files: config.guess config.sub`, then run the following:

```
brew install wget
wget -O config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
wget -O config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
./configure --with-tcl=/opt/homebrew/opt/tcl-tk/lib/tclConfig.sh --with-tk=/opt/homebrew/opt/tcl-tk/lib/tkConfig.sh
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

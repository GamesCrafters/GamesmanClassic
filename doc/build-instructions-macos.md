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

### Caveats of `tcl-tk`

From `brew info tcl-tk`:

> tcl-tk is keg-only, which means it was not symlinked into /usr/local, because tk installs some X11 headers and macOS provides an (older) Tcl/Tk.

We prefer making the `brew`-installed `tcl-tk` default. To so this, we add `tcl-tk` to the `$PATH` in the terminal init script.

If you're using `bash`, you may add the following line to the `.bash_profile` file in your home directory. If you're using `zsh`, you may add the following line to the `.zshenv` file instead.

```bash
export PATH="/usr/local/opt/tcl-tk/bin:$PATH"
```

After you're done editing & saving the init script, reopen a terminal session and the new init script should be in effect. (You can also use the `source` command to load the init script.)

## Building GamesmanClassic

Make sure you're in the directory of the repository. Then use the following script to build GamesmanClassic.

```bash
# Generate configure script
autoconf

# Configure with the installed tcl-tk
./configure --with-tcl=/usr/local/opt/tcl-tk/lib/tclConfig.sh --with-tk=/usr/local/opt/tcl-tk/lib/tkConfig.sh

# Build GamesmanClassic
make
```

## Running GamesmanClassic

```bash
# Must be in the bin/ directory first
cd bin

# Run the new GUI
./XGamesman.new
```

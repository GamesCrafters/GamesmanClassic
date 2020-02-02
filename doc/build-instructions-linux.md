# Build Instructions (macOS)

## Prerequisites

- `build-essential`
- `autoconf`
- `zlib1g-dev`
- `tcl-dev`
- `tk-dev`

You may install these prerequisites with the following command:

```
sudo apt install build-essential autoconf zlib1g-dev tcl-dev tk-dev
```

## Building GamesmanClassic

Make sure you're in the directory of the repository. Then use the following script to build GamesmanClassic.

```bash
# Generate configure script
autoconf

# Configure
./configure

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

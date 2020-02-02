# GamesmanClassic Configuration Options

If you want to change an option such as whether or not to build with graphics, then you may need to change the configuration options.

For a full list of options, type:

```bash
./configure --help
```

And a list of options available to you will be displayed.
To set any of the options listed, type them after `./configure` on the command line. This is shown:

```bash
./configure [option1 [option2 ...]]
# e.g.
./configure --with-aqua --with-tcl=/usr/lib/tclConfig.sh
```

Here are ones you may need to know about:

- `--disable-graphics`

    If you do not have Tcl/Tk (see http://www.tcl.tk), and you do not want to build gamesman with graphics, then use this option.

- `--with-aqua`

    If you have a Mac with Mac OS X as well as the macos distribution of Tcl/Tk, and you want the Aqua wish shell instead of X11, use this.
    
    Note: Currently, Aqua `wish` is plagued with bugs that cause certain graphics to not be properly displayed. The use of Aqua `wish` is not recommended.

- `--with-tcl`

    If the configure script fails to find the proper tclConfig.sh file, and you have tcl installed on your system, then you can tell it explicitly the file to find using this option.

- `--with-tk`

    Same as above, except regards the tkConfig.sh

- `--with-wish`

    Same as above, except regards the wish shell (usually `/usr/bin/wish` or `/usr/local/bin/wish`).

If changing the options does not fix the problem, then you should e-mail someone involved with gamescrafters and describe your situation.

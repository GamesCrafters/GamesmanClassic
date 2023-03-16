This misc folder contains files that were previously versioned in GamesmanClassic but may have low relevance to the GamesmanClassic software.
The original folder structure is preserved; files now inside the `/misc/src/` folder were originally located in `/src/`.

- `GamesmanLightServer/`

  Seems to be an attempt in Fall 2012 to write a game solution server in Java.
  Unsure what it does yet.

- `java/`

  Seems to be some work done in 2006.
  It has some resemblance to the GamesmanWeb project.
  Unsure what it does yet.

- `ppb/`

  An attempt to mimic the GamesmanClassic game's `--interact` interface in Python in Spring 2019.
  This way we can reuse the same mechanism of the Python server (`src/py/server.py`) to spin up Python processes the same way it starts subprocesses of game binaries.
  The `ppb` module is a quick and easy "hack" to put newly-solved games online.

- `src/js/`

  Seems to be a server written in Node that supports both GamesmanClassic and GamesmanJava in 2012.

- `src/scheme/`

  A solver written in Scheme in Spring 2007.

- `thrift/`

  Seems to be an attempt to write a thrift server for GamesmanClassic.
  Perhaps well coupled with GamesmanWeb but it's not in use as of Spring 2018.

- `doc/games/html/`

  Game documentations superceded by http://gamescrafters.berkeley.edu/games.php

- `doxygen-theme`

  CSS resources for making the Doxygen-generated documentation prettier (credit: https://jothepro.github.io/doxygen-awesome-css/).
  
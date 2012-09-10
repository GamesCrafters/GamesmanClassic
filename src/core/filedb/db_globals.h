/************************************************************************
**
** NAME:	db_globals.h
**
** DESCRIPTION:	Gamescrafters Database Globals Header File
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#ifndef GMCORE_DB_GLOBALS_H
#define GMCORE_DB_GLOBALS_H

#define GAMESDB_TRUE 1
#define GAMESDB_FALSE 0

#define GAMESDB_MAX_CHANCES 15 //this is in accord to the length of elements in buffer->chances

#define GAMESDB_CLUSTER_SIZE 10 //10 bits cluster for directory hierarchy

#define GAMESDB_MAX_FILENAME_LEN 256

#define GAMESDB_DEBUG 0

#define GAMESDB_GEOMETRY_FILENAME "geometry.dat"

#endif /* GMCORE_DB_GLOBALS_H */

/************************************************************************
**
** NAME:    quartodb.h
**
** DESCRIPTION:    Accessor functions for the Bit-Perfect Database.
**
** AUTHOR:    Cameron Cheung
**        GamesCrafters Research Group, UC Berkeley
**        Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:    2022-01-30
**
** LICENSE:    This file is part of GAMESMAN,
**        The Finite, Two-person Perfect-Information Game Generator
**        Released under the GPL:
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

#ifndef GMCORE_QUARTODB_H
#define GMCORE_QUARTODB_H

#include "db.h"

void    quartodb_init     (DB_Table*);
void    quartoDetailedPositionResponse(STRING board, char *positionStringBuffer);

#endif /* GMCORE_QUARTODB_H */

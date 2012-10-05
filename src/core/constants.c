/************************************************************************
**
** NAME:	constants.c
**
** DESCRIPTION:	Allocates memory for long string literals and other
**		constant declarations.
**		Mostly help/documentation.
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

#include "types.h"

/*
** Globals
*/

STRING kOpeningCredits =
        "\n"
        "  ____               http://gamescrafters.berkeley.edu  : A finite,  two-person\n"
        " / ___| __ _ _ __ ___   ___  ___ _ __ ___   __ _ _ __   : complete  information\n"
        "| |  _ / _` | '_ ` _ \\ / _ \\/ __| '_ ` _ \\ / _` | '_ \\  : game generator.  More\n"
        "| |_| | (_| | | | | | |  __/\\__ \\ | | | | | (_| | | | | : information?  Contact\n"
        " \\____|\\__,_|_| |_| |_|\\___||___/_| |_| |_|\\__,_|_| |_| : ddgarcia@berkeley.edu\n"
        "...............................................................................\n"
        "\n"
        "Welcome to GAMESMAN, version %s. Originally       (G)ame-independent\n"
        "written by Dan Garcia, it has undergone a series of       (A)utomatic       \n"
        "exhancements from 2001-present by GamesCrafters, the      (M)ove-tree       \n"
        "UC Berkeley Undergraduate Game Theory Research Group.     (E)xhaustive      \n"
        "                                                          (S)earch,         \n"
        "This program will determine the value of your game,       (M)anipulation    \n"
        "perform analysis, & provide an interface to play it.      (A)nd             \n"
        "Usage: %37s --help       (N)avigation      \n"
        "\n"
        "You are playing %s,\nwritten by %s. Enjoy!\n";

STRING kHelpValueBeforeEvaluation =
        "At this point, the computer does not know the value of the game\n"
        "because you have not chosen to EVALUATE it. Once you choose to\n"
        "do that, the value will be determined and shown to you. Then you \n"
        "can bring this help menu back up to find out what it means.";

STRING kHelpWhatIsGameValue =
        "A game VALUE is one of either WIN, LOSE, or TIE.  That is, if a game \n"
        "is played by two perfect opponents, the following ALWAYS happens:\n"
        "(A perfect opponent never makes a bad move, and only loses if there is\n"
        "NOTHING he/she can do about it.)\n\n"
        "WIN:\n\n"
        "The player who goes first always wins.\n\n"
        "LOSE:\n\n"
        "The player who goes first always loses.\n\n"
        "TIE:\n\n"
        "The player who goes first cannot force a win, but also cannot be\n"
        "forced into losing. The game always ends in a draw. It was a TIE\n"
        "game that caused a character from the movie 'War Games' to comment:\n\n"
        "The only way to win is not to play at all.\n\n"
        "The reason it is important to know the value of the game is that it\n"
        "determines whether going first or second is better. If the game is a\n"
        "WIN game, it is better to go first. Why? Well, theoretically, it is\n"
        "possible to win, regardless of what your opponent does. This applies\n"
        "equally to going second while playing a LOSE game. It doesn't mean that\n"
        "the person going first with a WIN game will ALWAYS win, it just says\n"
        "the potential exists. In GAMESMAN, the computer plays the part of the\n"
        "perfect opponent, but gives you, the human opponent the advantage. If\n"
        "you make a mistake, the computer pounces on you.";

STRING kHelpWhatIsEvaluation =
        "Evaluation is the process the computer undergoes to determine the value\n"
        "of the game. (See 'What is a game VALUE?', the previous topic, for more)\n"
        "The computer searches the MOVE-TREE (the connected tree of all the moves\n"
        "that are possible from a certain position that lead to other positions)\n"
        "recursively (Depth-First Search, for the computer literate) until it \n"
        "reaches a position which it considers a PRIMITIVE win, lose or draw.\n"
        "\n"
        "A PRIMITIVE position is defined by the game itself. For example, with\n"
        "Tic-tac-toe, if the board is full, that is considered a primitive TIE.\n"
        "If a position has 3-in-a-row of X, then that position is considered a \n"
        "primitive LOSE position, because that means player O, who is staring\n"
        "blankly at the board, has just LOST. Once a primitive position has been\n"
        "reached, the algorithm backtracks in an attempt to determine the value\n"
        "of THAT position. It uses the following scheme to figure out the value\n"
        "of a non-primitive position:\n"
        "\n"
        "A non-primitive WIN position is one in which THERE EXISTS AT LEAST ONE\n"
        "move that will give my opponent a LOSE position.\n"
        "\n"
        "A non-primitive LOSE position is one in which ALL MOVES that are possible\n"
        "will give my opponent a WIN position.\n"
        "\n"
        "A non-primitive TIE position is one in which there are NO MOVES which\n"
        "will give my opponent a LOSE position, but there are SOME MOVES which\n"
        "will give my opponent another TIE position. This translates to: \n"
        "'Well, I can't win, but at least my opponent can win either'.\n"
        "\n"
        "The algorithm continues until all positions that can be reached from\n"
        "the starting position (the blank board in Tic-Tac-Toe, for instance)\n"
        "have had their value determined, and in the meantime the computer has\n"
        "figured out PERFECT paths for itself that always force you to LOSE.";

STRING kHelpWhatArePredictions =
        "Predictions are the words in parenthesis you see when you are playing\n"
        "the game that 'predict' who will win and who will lose. Since the\n"
        "computer is a perfect opponent, if, while playing, it discovers it has\n"
        "a WIN position, there's nothing you can do, so the prediction is: 'Player\n"
        "will Lose' and 'Computer will Win'. However, if you have the WIN\n"
        "position, it's always possible for you to make a really silly move and\n"
        "give the COMPUTER the WIN position, so the prediction says: 'Player\n"
        "should Win' and 'Computer should Lose'.";

STRING kHelpWhatAreHints =
        "Hints are a list of moves that are VALUE-wise equivalent. For example,\n"
        "if you have a WIN position, then all the moves that will give your\n"
        "opponent a LOSE position are listed in the hints. If you have a LOSE\n"
        "position, ALL the possible moves are listed in the hints. The reason\n"
        "for this is explained in the help topic: 'What is EVALUATION?'. If you\n"
        "have a TIE position, the moves that give your opponent a TIE position\n"
        "are listed in the hints.";



STRING kHandleDefaultTextInputHelp =
        "\n"
        "Text Input Commands:\n"
        "-------------------\n"
        "?           : Brings up this list of Text Input Commands available\n"
        "p (or P)    : Brings up the (P)rint Menu\n"
        "c (or C)    : Brings up the (C)onfiguration Menu\n"
        "e (or E)    : Print Static (E)valuator value for this position\n"
        "u (or U)    : (U)ndo last move (not possible at beginning position)\n"
        "a (or A)    : (A)bort the game\n"
        "h (or H)    : (H)elp\n"
        "q (or Q)    : (Q)uit";

STRING kPrintMenu =
        "\n"
        "\tr)\t(R)eprint the position\n";


STRING kPrintMenuWithSolving =
        "\ts)\t(S)how the values of all possible moves\n"
        "\tv)\tPrint (V)isual Value History\n"
        "\tw)\tPrint Visual Value History (W)ith All Possible Moves\n";



STRING kPrintMenuEnd =
        "\tm)\tScript of (M)ove History\n"
        "\n\th)\t(H)elp\n\n"
        "\tb)\t(B)ack\n\n"
        "\tq)\t(Q)uit\n\n";


char *gValueString[] =
{
	"Undecided", "Win", "Lose", "Tie",
	"Win-Visited", "Lose-Visited",
	"Tie-Visited", "Undecided-Visited"
};

char gValueLetter[] = { 'U', 'W', 'L', 'T' };

STRING kSolveVersion = "2006.01.01";

STRING kCommandSyntaxHelp =
        "\nSyntax:\n"
        "%s  {--nodb | --newdb | --filedb | --numoptions | --curroption |\n"
        "\t--option <n> | --nobpdb | --2bit | --colldb | --univdb | --gps |\n"
        "\t--bottomup | --alpha-beta | --lowmem | --slicessolver | --schemes |\n"
        "\t--allschemes | --adjust | --noadjust | --solve [<n> | <all>] |\n"
        "\t--analyze [ <linkname> ] | --open | --visualize |\n"
        "\t--DoMove <args> <move> | --Primitive <args> | --PrintPosition <args> |\n"
        "\t--GenerateMoves <args>} | --lightplayer | --netDb | --hashCounting |\n"
        "\t--help}\n\n"
        "--export <filename>\t\t\tSolves the game (if needed) then exports to filename.\n"
        "--interact\t\t\tSolves the game (if needed) then enters server interaction mode.\n"
        "--nodb\t\t\tStarts game without loading or saving to the database.\n"
        "--newdb\t\t\tStarts game and clobbers the old database.\n"
        "--filedb\t\tStarts game with file-based database.\n"
        "--numoptions\t\tPrints the number of options.\n"
        "--curroption\t\tPrints the current option.\n"
        "--option <n>\t\tStarts game with the n option configuration.\n"
        "--nobpdb\t\tStarts game without using Bit Perfect Database.\n"
        "--2bit\t\t\tStarts game with two-bit solving enabled.\n"
        "--colldb\t\tStarts game with Collision based Database. Currently Experimental. \n"
#ifdef HAVE_GMP
        "--univdb\t\tStarts game with 2-Universal hash-based resizable database. \n"
#endif
        "--gps\t\t\tStarts game with global position solver enabled.\n"
        "--bottomup\n"
        "--alpha-beta\t\tStarts game with weak alpha-beta solver. \n"
        "--lowmem\t\tStarts game with low memory overhead solver enabled.\n"
        "--slicessolver\t\tWith bpdb turned on, the variable slice aware solver will be used (faster).\n"
        "--schemes\t\tWith bpdb turned on variable gaps compression will be used for saved dbs.\n"
        "--allschemes\n"
        "--adjust\t\tWith bpdb turned on, slice sizes will be grow and shrink to best-fit data.\n"
        "--noadjust\n"
        "--notiers\t\tStarts game with Tier-Gamesman Mode OFF by default.\n"
        "--notiermenu\t\tThis option disables the Tier-Gamesman solver menu, and auto-solves all tiers.\n"
        "--notierprint\t\tThis option disables the printing from the Tier-Gamesman solver menu.\n"
        "--solve [<n> | <all>]\tSolves game with the n option configuration.\n"
        "\t\t\tTo solve all option configurations of game, use <all>.\n"
        "\t\t\tIf <n> and <all> are ommited, it will solve the default\n"
        "\t\t\tconfiguration.\n"
        "\t\t\tExamples:\n"
        "\t\t\t%s --solve\n"
        "\t\t\t%s --solve 2\n"
        "\t\t\t%s --solve all\n"
        "--analyze\t\tCreates the analysis directory with info on all variants\n"
        "--open\t\t\tStarts game with Open Positions solving enabled.\n"
        "--visualize\t\tTurns on automatic visualization.\n"
        "--DoMove <pos> <move>\tDoes the move on the position.\n"
        "--Primitive <pos>\tChecks whether position is a primitive.\n"
        "--PrintPosition <pos>\tPrints the ASCII representation of the position.\n"
        "--GenerateMoves <pos>\tGenerates all possible moves from position.\n"
        "--lightplayer\t\tHints the database to minimize memory usage.\n"
        "--netDb\t\t\tStarts game with the network database.\n"
        "--hashCounting\t\tStarts the generic-hash counting tool instead of the game.\n"
        "--hashtable_buckets\t(advanced) Sets the total number of buckets in any hashtables used.\n"
        "--withPen <file>\tStarts game with Anoto Pen support, reading data from <file> (with GUI only)\n"
        "--penDebug\t\tEnables Anoto Pen log messages / data saving to 'bin/pen/' (with GUI only)\n\n";


#ifndef GM_TYPES_H

/*
** Simple (non-aggregate)
*/

#ifdef uint64_t
typedef	uint64_t		POSITION;
#else
typedef unsigned long		POSITION;
#endif

typedef	int			VALUE;
typedef	int			MOVE;
typedef	int			REMOTENESS;
typedef	char*			STRING;
typedef	void*			GENERIC_PTR;
typedef	enum { Undo, Abort,
	       Continue, Move }	USERINPUT;

#ifndef BOOLEAN
#ifdef WIN32
typedef	unsigned char		BOOLEAN;
#else /* WIN32 */
typedef	int			BOOLEAN;
#endif
#endif


/*
** Structures
*/

typedef struct PosList_struct {
	POSITION		position;
	struct PosList_struct*	next;
} POSITIONLIST;

typedef struct MoveList_struct {
	MOVE			move;
	struct MoveList_struct*	next;
} MOVELIST;

typedef struct SL_struct {
	struct SL_struct*	next;
	char			string[0];
} STRINGLIST;

typedef struct {
	int			tier_count;
	POSITIONLIST**		tiers;
} VALUE_MOVES;

typedef enum { VAL_NONE, VAL_STRING, VAL_VOIDP, VAL_INT, VAL_POS } table_type;
typedef struct {
	void			(*delete)(void*);
	table_type		type;
	union {
		POSITION	pos;
		int		num;
		void*		ptr;
		STRING		str;
	}			value;
} table_value;

typedef struct table_entry {
	STRING			key;
	table_value		value;
	
	struct table_entry*	next;
} *TABLE;

typedef	TABLE			table_iter;


/*
** Classes
*/

typedef struct {
	struct GAME_struct*	game;
	struct DB_struct*	db;
	struct SOLVER_struct*	solver;
} TRIAD;

typedef struct GAME_struct {
	TRIAD*			triad;
	void*			internal;
	TABLE			properties;
	
	int			(*init)(struct GAME_struct*);
	int			(*free)(struct GAME_struct*);
	
	POSITION		(*do_move)(struct GAME_struct*, POSITION, MOVE, int*);
	VALUE			(*primitive)(struct GAME_struct*, POSITION);
	MOVELIST*		(*generate_moves)(struct GAME_struct*, POSITION, int*);
	POSITIONLIST*		(*generate_parents)(struct GAME_struct*, POSITION);
	int			(*whose_turn)(struct GAME_struct*, POSITION);
	
	struct TEXTGAME*	text_functions;
	struct TCLGAME*		tcl_functions;
} GAME;

struct TEXTGAME {
	void			(*debug_menu)(GAME*);
	void			(*game_specific_menu)(GAME*);
	void			(*print_computers_move)(GAME*, MOVE, STRING);
	void			(*print_position)(GAME*, POSITION, STRING, BOOLEAN);
	USERINPUT		(*get_and_print_players_move)(GAME*, POSITION, MOVE, STRING);
	BOOLEAN			(*valid_text_input)(GAME*, STRING);
	MOVE			(*convert_text_input_to_move)(GAME*, STRING);
	void			(*print_move)(GAME*, MOVE);
};

struct TCLGAME {
	void			(*tcl_game_options)(GAME*, int[]);
	int			(*tcl_game_init)(GAME*, Tcl_Interp*, Tk_Window);
};

typedef struct DB_struct {
	TRIAD*			triad;
	void*			internal;
	TABLE			properties;
	TABLE			content_properties;
	
	int			(*init)(struct DB_struct*, POSITION, int, TABLE);
	int			(*free)(struct DB_struct*);
	int			(*get)(struct DB_struct*, POSITION, void*);
	int			(*put)(struct DB_struct*, POSITION, void*);
	int			(*putn)(struct DB_struct*, POSITION, void*, int);
//	int			(*dumpn)(struct DB_struct*, POSITION*, int);
	int			(*transfer)(struct DB_struct*, struct DB_struct*);
} DATABASE;

typedef struct SOLVER_struct {
	TRIAD*			triad;
	void*			internal;
	TABLE			properties;
	
	int			(*solve)(struct SOLVER_struct*);
	int			(*free)(struct SOLVER_struct*);
	REMOTENESS		(*get_remoteness)(struct SOLVER_struct*, POSITION);
	VALUE			(*get_value)(struct SOLVER_struct*, POSITION);
	VALUE_MOVES		(*get_value_moves)(struct SOLVER_struct*, POSITION);
} SOLVER;

typedef struct {
	STRING			class_name;
	TABLE			properties;
	
	SOLVER*			(*create)(GAME*, DATABASE*);
} SOLVER_CLASS;

typedef struct {
	STRING			class_name;
	TABLE			properties;
	
	DATABASE*		(*create)(STRING);
} DATABASE_CLASS;

typedef struct {
	STRING			class_name;
	TABLE			properties;
	
	GAME*			(*create)(int);
} GAME_CLASS;


/*
** Constants
*/

#ifndef TRUE
#define	TRUE			1
#endif
#ifndef	FALSE
#define	FALSE			0
#endif


#endif /* GM_TYPES_H */

#include <Python.h>
#include "gamesman.h"

/* Interface to Tk not supported yet */
void*    gGameSpecificTclInit = NULL;


BOOLEAN kGameSpecificMenu;
BOOLEAN kPartizan;
BOOLEAN kDebugMenu;
BOOLEAN kTieIsPossible;
BOOLEAN kLoopy;
BOOLEAN kDebugDetermineValue;
STRING kAuthorName;
STRING kGameName;
STRING kHelpGraphicInterface;
STRING kHelpTextInterface;
STRING kHelpOnYourTurn;
STRING kHelpStandardObjective;
STRING kHelpReverseObjective;
STRING kHelpTieOccursWhen;
STRING kHelpExample;
STRING kDBName;
POSITION kBadPosition;
POSITION gInitialPosition;
POSITION gNumberOfPositions;

#define PyPosition_FromPosition(x) PyLong_FromUnsignedLongLong(x)
#define PyPosition_AsPosition(x) PyLong_AsUnsignedLongLong(x)

#define PyMove_FromMove(x) PyInt_FromLong(x)
#define PyMove_AsMove(x) PyInt_AsLong(x)

#define PyBoolean_FromBoolean(x) (x == TRUE) ? Py_True : Py_False

void importBOOLEAN(void *to, PyObject *from) {
	*((BOOLEAN *)to) = (from == Py_True) ? TRUE : FALSE;
}

PyObject *exportBOOLEAN(BOOLEAN from) {
	return ( from == TRUE ) ? Py_True : Py_False;
}

void importPOSITION(void *to, PyObject *from) {
	*((POSITION *)to) = PyLong_AsUnsignedLongLong(from);
}

PyObject *exportPOSITION(POSITION from) {
	return PyLong_FromUnsignedLongLong(from);
}

void importSTRING(void *to, PyObject *from) {
	*((STRING *)to) = PyString_AsString(from);
}

static PyObject *callback = NULL;

/* Should be in gamesman.h */
int gamesman_main(int argc, char *argv[]);

void *PrintError() {

	PyErr_Print();
	exit(1);

}

#define verify(value) \
	value == 0 && PrintError() || value

static PyObject *initialize(PyObject *self, PyObject *args) {
	PyObject *result = NULL;
	PyObject *arglist;
	PyObject *handler;

	if (PyArg_ParseTuple(args, "OO", &arglist, &handler)) {

		if (!PyCallable_Check(handler)) {
			PyErr_SetString(PyExc_TypeError, "second argument must be callable");
			return NULL;
		} else {
			Py_XINCREF(handler);
			Py_XDECREF(callback);
			callback=handler;
		}

		if (!PyList_Check(arglist)) {
			PyErr_SetString(PyExc_TypeError, "first argument must be a list");
			return NULL;
		} else {

			int index, argc = PyList_Size(arglist);
			char *argv[argc];

			for (index = 0; index < argc; index++) {
				if (!PyArg_Parse(PyList_GetItem(arglist, index), "s", argv + index)) {
					return NULL;
				}
			}
			result = Py_BuildValue("i", gamesman_main(argc, argv));
		}
	}
	return result;

}

static PyObject *export (PyObject *self, PyObject *args, PyObject *keywords) {

	int index;
	static struct {
		char *keyword;
		void *variable;
		void (*import)(void *, PyObject *);
	} varmap[] = {
		{"kGameSpecificMenu", &kGameSpecificMenu, &importBOOLEAN},
		{"kPartizan", &kPartizan, &importBOOLEAN},
		{"kDebugMenu", &kDebugMenu, &importBOOLEAN},
		{"kTieIsPossible", &kTieIsPossible, &importBOOLEAN},
		{"kLoopy", &kLoopy, &importBOOLEAN},
		{"kDebugDetermineValue", &kDebugDetermineValue, &importBOOLEAN},
		{"kAuthorName", &kAuthorName, &importSTRING},
		{"kGameName", &kGameName, &importSTRING},
		{"kHelpGraphicInterface", &kHelpGraphicInterface, &importSTRING},
		{"kHelpTextInterface", &kHelpTextInterface, &importSTRING},
		{"kHelpOnYourTurn", &kHelpOnYourTurn, &importSTRING},
		{"kHelpStandardObjective", &kHelpStandardObjective, &importSTRING},
		{"kHelpReverseObjective", &kHelpReverseObjective, &importSTRING},
		{"kHelpTieOccursWhen", &kHelpTieOccursWhen, &importSTRING},
		{"kHelpExample", &kHelpExample, &importSTRING},
		{"kDBName", &kDBName, &importSTRING},
		{"kBadPosition", &kBadPosition, &importPOSITION},
		{"gInitialPosition", &gInitialPosition, &importPOSITION},
		{"gNumberOfPositions", &gNumberOfPositions, &importPOSITION}
	};

	for (index = 0; index < (sizeof(varmap)/sizeof(*varmap)); index++ ) {
		PyObject *key = PyString_FromString(varmap[index].keyword);
		PyObject *value = PyDict_GetItem(keywords, key);
		if (value) {
			varmap[index].import( varmap[index].variable, value );
		}
		Py_DECREF(key);
	}

	return Py_None;
}

static PyMethodDef functions[] = {
	{"initialize",  initialize, METH_VARARGS, "Initializes callback handler and calls main"},
	{"export",  (PyCFunction) export, METH_VARARGS | METH_KEYWORDS, "Exports keyworded variables"},
	{NULL, 0}
};

PyMODINIT_FUNC initpygamesman() {
	(void) Py_InitModule("pygamesman", functions);
}

PyAPI_FUNC(PyObject *) call(PyObject *arglist) {

	PyObject *result;

	result = PyEval_CallObject(callback, arglist);
	Py_DECREF(arglist);

	if ( result == NULL ) {
		PyErr_Print();
		exit(1);
	}
	return result;
}

void InitializeGame() {
	Py_DECREF(call(Py_BuildValue("(s)", "InitializeGame")));
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	PyObject *py_position, *py_playerName, *py_usersTurn;

	py_position = exportPOSITION(position);
	py_playerName = PyString_FromString(playerName);
	py_usersTurn = exportBOOLEAN(usersTurn);
	Py_DECREF(call(Py_BuildValue("(sOOO)", "PrintPosition", py_position, py_playerName, py_usersTurn)));
	Py_DECREF(py_position);
	Py_DECREF(py_playerName);

}

MOVELIST *GenerateMoves(POSITION position) {
	PyObject *py_position;
	PyObject *py_movelist;
	MOVELIST *movelist = NULL;
	int index;

	py_position = PyPosition_FromPosition(position);
	py_movelist = call(Py_BuildValue("(sO)", "GenerateMoves", py_position));

	if (!PyList_Check(py_movelist)) {
		PyErr_SetString(PyExc_TypeError, "returned value must be list");
		PyErr_Print();
		exit(1);
	}

	for (index = 0; index < PyList_Size(py_movelist); index++ ) {

		MOVE move;
		verify(PyArg_Parse(PyList_GetItem(py_movelist, index), "i", &move));
		movelist = CreateMovelistNode(move, movelist);

	}

	Py_DECREF(py_movelist);
	return movelist;

}

VALUE Primitive(POSITION position) {
	PyObject *py_position, *py_value;
	VALUE value;

	py_position = PyPosition_FromPosition(position);
	py_value = call(Py_BuildValue("(sO)", "Primitive", py_position));
	verify(PyArg_Parse(py_value, "i", &value));

	Py_DECREF(py_position);
	Py_DECREF(py_value);

	return value;
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
	USERINPUT HandleDefaultTextInput();
	USERINPUT input = Continue;
	PyObject *py_position, *py_playerName, *py_funcall;

	py_position = PyPosition_FromPosition(position);
	py_playerName = PyString_FromString(playerName);
	py_funcall = Py_BuildValue("(sOO)", "PrintPrompt", py_position, py_playerName);

	while ( input == Continue ) {

		Py_INCREF(py_funcall);
		Py_DECREF(call(py_funcall));
		input = HandleDefaultTextInput(position, move, playerName);

	}

	Py_DECREF(py_position);
	Py_DECREF(py_playerName);
	Py_DECREF(py_funcall);

	return input;
}

MOVE ConvertTextInputToMove(STRING input) {
	PyObject *py_input, *py_move;
	MOVE move;

	py_input = PyString_FromString(input);
	py_move = call(Py_BuildValue("(sO)", "ConvertTextInputToMove", py_input));
	verify(PyArg_Parse(py_move, "i", &move));

	Py_DECREF(py_input);
	Py_DECREF(py_move);

	return move;

}

POSITION GetInitialPosition() {
	return 0;
}

void PrintMove(MOVE move) {
	PyObject *py_move;

	py_move = PyInt_FromLong(move);
	Py_DECREF(call(Py_BuildValue("(sO)", "PrintMove", py_move)));
	Py_DECREF(py_move);
}

void PrintComputersMove(MOVE move, STRING computerName) {
	PyObject *py_move, *py_computerName;

	py_move = PyInt_FromLong(move);
	py_computerName = PyString_FromString(computerName);
	Py_DECREF(call(Py_BuildValue("(sOO)", "PrintComputersMove", py_move, py_computerName)));
	Py_DECREF(py_move);
	Py_DECREF(py_computerName);
}

int getOption () {
	return 0;
}

POSITION DoMove(POSITION position, MOVE move) {

	PyObject *py_position, *py_move, *py_newposition;
	POSITION newposition;

	py_position = PyPosition_FromPosition(position);
	py_move = PyMove_FromMove(move);
	py_newposition = call(Py_BuildValue("(sOO)", "DoMove", py_position, py_move));

	importPOSITION(&newposition, py_newposition);

	Py_DECREF(py_position);
	Py_DECREF(py_move);
	Py_DECREF(py_newposition);

	return newposition;

}

void GameSpecificMenu(){
}

void DebugMenu(){
}

void setOption(int option){
}

BOOLEAN ValidTextInput(STRING input){
	PyObject *py_input;
	PyObject *py_valid;
	BOOLEAN valid;

	py_input = PyString_FromString(input);
	py_valid = call(Py_BuildValue("(sO)", "ValidTextInput", py_input));

	if (!PyBool_Check(py_valid)) {
		exit(1);
	}

	importBOOLEAN(&valid, py_valid);
	return valid;
}

int NumberOfOptions(){
	return 0;
}


#ifndef GM_TEXTUI_H
#define GM_TEXTUI_H

void		HitAnyKeyToContinue	();
char		GetMyChar		();
void		BadMenuChoice		();

void		Menus			();
void		HelpMenus		();

USERINPUT	HandleDefaultTextInput	(POSITION pos, MOVE* move, STRING name);
void		GetMyString		(char* name, int size, BOOLEAN eatFirst, BOOLEAN preserveCR);

void		showStatus		(int value);



#endif /* GM_TEXTUI_H */

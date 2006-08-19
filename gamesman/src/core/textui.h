
#ifndef GMCORE_TEXTUI_H
#define GMCORE_TEXTUI_H

void		HitAnyKeyToContinue	();
char		GetMyChar		();
int		GetMyInt		();
void		GetMyHelper		(char *format, GENERIC_PTR target);
void		GetMy			(char *format, GENERIC_PTR target, int length);
void		BadMenuChoice		();

void		Menus			();
void		HelpMenus		();

USERINPUT	HandleDefaultTextInput	(POSITION pos, MOVE* move, STRING name);
void		GetMyString		(char* name, int size, BOOLEAN eatFirst, BOOLEAN preserveCR);

void		showStatus		(STATICMESSAGE msg);



#endif /* GMCORE_TEXTUI_H */

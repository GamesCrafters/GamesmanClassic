
#ifndef GMCORE_TEXTUI_H
#define GMCORE_TEXTUI_H

void            HitAnyKeyToContinue     (void);
char            GetMyChar               (void);
int             GetMyInt                (void);
unsigned int    GetMyUInt               (void);
void            GetMyHelper             (char *format, GENERIC_PTR target);
void            GetMy                   (char *format, GENERIC_PTR target, int length, BOOLEAN keepSpaces);
void            GetMyStr                (STRING str, int len);
void            BadMenuChoice           (void);

void            Menus                   (STRING executableName);
void            HelpMenus               (void);

USERINPUT       HandleDefaultTextInput  (POSITION pos, MOVE* move, STRING name);
void            showStatus              (STATICMESSAGE msg);
void            showDBLoadingStatus     (STATICMESSAGE msg);

#endif /* GMCORE_TEXTUI_H */


typedef unsigned int I_Modifiers;
typedef enum
{
    I_Modifiers_Ctrl  = Bit(0),
    I_Modifiers_Shift = Bit(1),
    I_Modifiers_Alt   = Bit(2),
} KeyModifiers_ENUM;

#define KeyDef(name, string) I_Key_ ## name ## ,
typedef enum
{
#include "graphics__keylist.h"
} I_Key;

Function S8 I_S8FromKey (I_Key key);

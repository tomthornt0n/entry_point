typedef unsigned int KeyModifiers;
typedef enum
{
    KeyModifiers_Ctrl  = Bit(0),
    KeyModifiers_Shift = Bit(1),
    KeyModifiers_Alt   = Bit(2),
} KeyModifiers_ENUM;

#define KeyDef(name, string) Key_ ## name ## ,
typedef enum
{
#include "base__keylist.h"
} Key;

static S8 S8FromKey(Key key);

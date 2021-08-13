static S8
S8FromKey(Key key)
{
    S8 result;
    
    static S8 strings[Key_MAX + 1] =
    {
#define KeyDef(name, string) { .buffer = string, .len = sizeof(string) - 1 },
#include "base__keylist.h"
    };
    
    if(Key_NONE <= key && key <= Key_MAX)
    {
        result = strings[key];
    }
    else
    {
        result = S8("ERROR - invalid key");
    }
    
    return result;
}

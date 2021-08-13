#undef STB_SPRINTF_IMPLEMENTATION
#include "external/stb_sprintf.h"

//~NOTE(tbt): string types

//-NOTE(tbt): utf8 ecnoded strings
typedef struct
{
 char *buffer;
 size_t len;
} String8;
typedef String8 S8;
#define S8(s) ((S8){ s, sizeof(s) - 1 })

//-NOTE(tbt): utf16 ecnoded strings
typedef struct
{
 wchar_t *buffer;
 size_t len;
} String16;
typedef String16 S16;
#define S16(s) ((S16){ WidenString(s), (sizeof(WidenString(s)) - 2) / 2 })

//-NOTE(tbt): utf132 ecnoded strings
typedef struct
{
 unsigned int *buffer;
 size_t len;
} String32;
typedef String32 S32;

//-NOTE(tbt): helper to 'unravel' a string into args for a "%.*s" format string
#define Unravel(s) (int)(s).len, (s).buffer

//~NOTE(tbt): character utilites

static Bool CharIsSymbol(unsigned int c);
static Bool CharIsSpace(unsigned int c);
static Bool CharIsNumber(unsigned int c);
static Bool CharIsUppercase(unsigned int c);
static Bool CharIsLowercase(unsigned int c);
static Bool CharIsLetter(unsigned int c);
static Bool CharIsAlphanumeric(unsigned int c);

static unsigned int CharLowerFromUpper(unsigned int c);
static unsigned int CharUpperFromLower(unsigned int c);

//~NOTE(tbt): c-string helpers

static size_t CStringCalculateUTF8Length(char *cstring);
static size_t CStringCalculateUTF16Length(wchar_t *cstring);
static S8 CStringAsS8(char *cstring);
static S16 CStringAsS16(wchar_t *cstring);

//~NOTE(tbt): unicode conversions

typedef struct
{
 unsigned int codepoint;
 int advance; // NOTE(tbt): in characters
} UTFConsume;

static Bool IsUTF8ContinuationByte(S8 string, int index);

// NOTE(tbt): from utf8
static UTFConsume CodepointFromUTF8(S8 string, int index);
static S16 S16FromS8(M_Arena *arena, S8 string);
static S32 S32FromS8(M_Arena *arena, S8 string);

// NOTE(tbt): from utf16
static UTFConsume CodepointFromUTF16(S16 string, int index);
static S8 S8FromS16(M_Arena *arena, S16 string);
static S32 S32FromS16(M_Arena *arena, S16 string);

// NOTE(tbt): from utf32
static S8 UTF8FromCodepoint(M_Arena *arena, unsigned int codepoint);
static S16 UTF16FromCodepoint(M_Arena *arena, unsigned int codepoint);
static S8 S8FromS32(M_Arena *arena, S32 string);
static S16 S16FromS32(M_Arena *arena, S32 string);

//~NOTE(tbt): string operations

static S8 S8Clone(M_Arena *arena, S8 string);

static S8 S8FromFmtV(M_Arena *arena, char *fmt, va_list args);
static S8 S8FromFmt(M_Arena *arena, char *fmt, ...);
typedef enum
{
 S8CompareFlags_RightSideSloppy = Bit(0), // NOTE(tbt): if the lengths are not equal, compare up to the length of the shortest string
 S8CompareFlags_CaseInsensitive = Bit(1),
} S8CompareFlags;
static Bool S8Compare(S8 a, S8 b, S8CompareFlags flags);

//~NOTE(tbt): string lists

typedef struct S8Node S8Node;
struct S8Node
{
 S8 string;
 S8Node *next;
};

typedef struct
{
 S8Node *first;
 S8Node *last;
} S8List;

// NOTE(tbt): clones the string to the arena and allocates a new node in the arena
static void S8ListPush(M_Arena *arena, S8List *list, S8 string);
static void S8ListAppend(M_Arena *arena, S8List *list, S8 string);
// NOTE(tbt): simple inserts the passed node into the list
static void S8ListPushExplicit(S8List *list, S8Node *string);
static void S8ListAppendExplicit(S8List *list, S8Node *string);

static S8 S8ListJoin(M_Arena *arena, S8List list);

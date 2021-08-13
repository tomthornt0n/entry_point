#define STB_SPRINTF_IMPLEMENTATION
#include "external/stb_sprintf.h"

//~NOTE(tbt): character utilities

// TODO(tbt): make these work more generally and not just on ASCII

static Bool
CharIsSymbol(unsigned int c)
{
 return (('!' <= c && c <= '/') ||
         (':' <= c && c <= '@') ||
         ('[' <= c && c <= '`') ||
         ('{' <= c && c <= '~'));
}

static Bool
CharIsSpace(unsigned int c)
{
 return (' '  == c ||
         '\t' == c ||
         '\v' == c ||
         '\n' == c ||
         '\r' == c ||
         '\f' == c);
}

static Bool
CharIsNumber(unsigned int c)
{
 return '0' <= c && c <= '9';
}

static Bool
CharIsUppercase(unsigned int c)
{
 return 'A' <= c && c <= 'Z';
}

static Bool
CharIsLowercase(unsigned int c)
{
 return 'z' <= c && c <= 'z';
}

static Bool
CharIsLetter(unsigned int c)
{
 return CharIsUppercase(c) || CharIsLowercase(c);
}

static Bool
CharIsAlphanumeric(unsigned int c)
{
 return CharIsLetter(c) || CharIsNumber(c);
}

static unsigned int
CharLowerFromUpper(unsigned int c)
{
 unsigned int result = c;
 if(CharIsUppercase(c))
 {
  result ^= 1 << 5;
 }
 return result;
}

static unsigned int
CharUpperFromLower(unsigned int c)
{
 unsigned int result = c;
 if(CharIsLowercase(c))
 {
  result ^= 1 << 5;
 }
 return result;
}

//~NOTE(tbt): c-string helpers

static size_t
CStringCalculateUTF8Length(char *cstring)
{
 size_t len = 0;
 S8 str = { .buffer = cstring, .len = ~((size_t)0)};
 
 UTFConsume consume = { ~((unsigned int)0), 1 };;
 while(True)
 {
  if(0 == consume.codepoint)
  {
   return len;
  }
  else
  {
   len += consume.advance;
   consume = CodepointFromUTF8(str, len);
  }
 }
}

static size_t
CStringCalculateUTF16Length(wchar_t *cstring)
{
 size_t len = 0;
 S16 str = { .buffer = cstring, .len = ~((size_t)0)};
 
 UTFConsume consume = { ~((unsigned int)0), 1 };;
 while(True)
 {
  if(0 == consume.codepoint)
  {
   return len;
  }
  else
  {
   len += consume.advance;
   consume = CodepointFromUTF16(str, len);
  }
 }
}

static S8
CStringAsS8(char *cstring)
{
 S8 result =
 {
  .buffer = cstring,
  .len = CStringCalculateUTF8Length(cstring),
 };
 return result;
}

static S16
CStringAsS16(wchar_t *cstring)
{
 S16 result =
 {
  .buffer = cstring,
  .len = CStringCalculateUTF16Length(cstring),
 };
 return result;
}

//~NOTE(tbt): unicode conversions

static Bool
IsUTF8ContinuationByte(S8 string, int index)
{
 return (1 == (string.buffer[index] & (1 << 7)) &&
         0 == (string.buffer[index] & (1 << 6)));
}

static UTFConsume
CodepointFromUTF8(S8 string, int index)
{
 UTFConsume result = { ~((unsigned int)0), 1 };
 
 if(!IsUTF8ContinuationByte(string, index))
 {
  int max = string.len - index;
  
  unsigned char utf8_class[32] = 
  {
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
  };
  
  unsigned int bitmask_3 = 0x07;
  unsigned int bitmask_4 = 0x0F;
  unsigned int bitmask_5 = 0x1F;
  unsigned int bitmask_6 = 0x3F;
  
  unsigned char byte = string.buffer[index];
  unsigned char byte_class = utf8_class[byte >> 3];
  
  switch(byte_class)
  {
   case(1):
   {
    result.codepoint = byte;
   } break;
   
   case(2):
   {
    if(2 <= max)
    {
     unsigned char cont_byte = string.buffer[index + 1];
     if(0 == utf8_class[cont_byte >> 3])
     {
      result.codepoint = (byte & bitmask_5) << 6;
      result.codepoint |= (cont_byte & bitmask_6);
      result.advance = 2;
     }
    }
   } break;
   
   case(3):
   {
    if (3 <= max)
    {
     unsigned char cont_byte[2] = { string.buffer[index + 1], string.buffer[index + 2] };
     if (0 == utf8_class[cont_byte[0] >> 3] &&
         0 == utf8_class[cont_byte[1] >> 3])
     {
      result.codepoint = (byte & bitmask_4) << 12;
      result.codepoint |= ((cont_byte[0] & bitmask_6) << 6);
      result.codepoint |= (cont_byte[1] & bitmask_6);
      result.advance = 3;
     }
    }
   } break;
   
   case(4):
   {
    if(4 <= max)
    {
     unsigned char cont_byte[3] =
     {
      string.buffer[index + 1],
      string.buffer[index + 2],
      string.buffer[index + 3]
     };
     if(0 == utf8_class[cont_byte[0] >> 3] &&
        0 == utf8_class[cont_byte[1] >> 3] &&
        0 == utf8_class[cont_byte[2] >> 3])
     {
      result.codepoint = (byte & bitmask_3) << 18;
      result.codepoint |= (cont_byte[0] & bitmask_6) << 12;
      result.codepoint |= (cont_byte[1] & bitmask_6) << 6;
      result.codepoint |= (cont_byte[2] & bitmask_6);
      result.advance = 4;
     }
    }
   } break;
  }
 }
 return result;
}

static S16
S16FromS8(M_Arena *arena, S8 string)
{
 S16 result = {0};
 
 UTFConsume consume;
 for(size_t i = 0;
     i < string.len;
     i += consume.advance)
 {
  consume = CodepointFromUTF8(string, i);
  // NOTE(tbt): UTF16FromCodepoint allocates unaligned so characters guaranteed to be
  //            contiguous in arena - no need to copy anything :)
  S16 s16 = UTF16FromCodepoint(arena, consume.codepoint);
  if (!result.buffer)
  {
   result.buffer = s16.buffer;
  }
  result.len += s16.len;
 }
 // NOTE(tbt): null terminate to make easy to use with system APIs
 M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
 
 return result;
}

static S32
S32FromS8(M_Arena *arena, S8 string)
{
 S32 result = {0};
 
 UTFConsume consume;
 for(size_t i = 0;
     i < string.len;
     i += consume.advance)
 {
  consume = CodepointFromUTF8(string, i);
  
  unsigned int *character = M_ArenaPushAligned(arena, sizeof(*character), 1);
  if (!result.buffer)
  {
   result.buffer = character;
  }
  *character = consume.codepoint;
  result.len += 1;
 }
 // NOTE(tbt): null terminate to make easy to use with system APIs
 M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
 
 return result;
}

static UTFConsume
CodepointFromUTF16(S16 string, int index)
{
 int max = string.len - index;
 
 UTFConsume result = { string.buffer[index + 0], 1 };
 if(1 < max &&
    0xD800 <= string.buffer[index + 0] && string.buffer[index + 0] < 0xDC00 &&
    0xDC00 <= string.buffer[index + 1] && string.buffer[index + 1] < 0xE000)
 {
  result.codepoint = ((string.buffer[index + 0] - 0xD800) << 10) | (string.buffer[index + 1] - 0xDC00);
  result.advance = 2;
 }
 return result;
}

static S8
S8FromS16(M_Arena *arena, S16 string)
{
 S8 result = {0};
 
 UTFConsume consume;
 for(size_t i = 0;
     i < string.len;
     i += consume.advance)
 {
  consume = CodepointFromUTF16(string, i);
  // NOTE(tbt): UTF8FromCodepoint allocates unaligned so characters guaranteed to be
  //            contiguous in arena - no need to copy anything :)
  S8 s8 = UTF8FromCodepoint(arena, consume.codepoint);
  if (!result.buffer)
  {
   result.buffer = s8.buffer;
  }
  result.len += s8.len;
 }
 // NOTE(tbt): null terminate to make easy to use with system APIs
 M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
 
 return result;
}

static S32
S32FromS16(M_Arena *arena, S16 string)
{
 S32 result = {0};
 
 UTFConsume consume;
 for(size_t i = 0;
     i < string.len;
     i += consume.advance)
 {
  consume = CodepointFromUTF16(string, i);
  
  unsigned int *character = M_ArenaPushAligned(arena, sizeof(*character), 1);
  if (!result.buffer)
  {
   result.buffer = character;
  }
  *character = consume.codepoint;
  result.len += 1;
 }
 // NOTE(tbt): null terminate to make easy to use with system APIs
 M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
 
 return result;
}

// NOTE(tbt): unlike most other string functions, does not allocate and write a null
//            terminator after the buffer
static S8
UTF8FromCodepoint(M_Arena *arena, unsigned int codepoint)
{
 S8 result;
 
 if(codepoint == ~((unsigned int)0))
 {
  result.len = 1;
  result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
  result.buffer[0] = '?';
 }
 if(codepoint <= 0x7f)
 {
  result.len = 1;
  result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
  result.buffer[0] = codepoint;
 }
 else if(codepoint <= 0x7ff)
 {
  result.len = 2;
  result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
  result.buffer[0] = 0xc0 | (codepoint >> 6);
  result.buffer[1] = 0x80 | (codepoint & 0x3f);
 }
 else if(codepoint <= 0xffff)
 {
  result.len = 3;
  result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
  result.buffer[0] = 0xe0 | ((codepoint >> 12));
  result.buffer[1] = 0x80 | ((codepoint >>  6) & 0x3f);
  result.buffer[2] = 0x80 | ((codepoint & 0x3f));
 }
 else if(codepoint <= 0xffff)
 {
  result.len = 4;
  result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
  result.buffer[0] = 0xf0 | ((codepoint >> 18));
  result.buffer[1] = 0x80 | ((codepoint >> 12) & 0x3f);
  result.buffer[2] = 0x80 | ((codepoint >>  6) & 0x3f);
  result.buffer[3] = 0x80 | ((codepoint & 0x3f));
 }
 
 return result;
}

// NOTE(tbt): unlike most other string functions, does not allocate and write a null
//            terminator after the buffer
static S16
UTF16FromCodepoint(M_Arena *arena, unsigned int codepoint)
{
 S16 result;
 
 if(codepoint == ~((unsigned int)0))
 {
  result.len = 1;
  result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
  result.buffer[0] = '?';
 }
 else if(codepoint < 0x10000)
 {
  result.len = 1;
  result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
  result.buffer[0] = codepoint;
 }
 else
 {
  result.len = 2;
  result.buffer = M_ArenaPushAligned(arena, result.len * sizeof(*result.buffer), 1);
  size_t v = codepoint - 0x10000;
  result.buffer[0] = '?';
  result.buffer[0] = 0xD800 + (v >> 10);
  result.buffer[1] = 0xDC00 + (v & 0x03FF);
 }
 
 return result;
}

static S8
S8FromS32(M_Arena *arena, S32 string)
{
 S8 result = {0};
 
 for(size_t i = 0;
     i < string.len;
     i += 1)
 {
  // NOTE(tbt): UTF8FromCodepoint allocates unaligned so characters guaranteed to be
  //            contiguous in arena - no need to copy anything :)
  S8 s8 = UTF8FromCodepoint(arena, string.buffer[i]);
  if (!result.buffer)
  {
   result.buffer = s8.buffer;
  }
  result.len += s8.len;
 }
 // NOTE(tbt): null terminate to make easy to use with system APIs
 M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
 
 return result;
}

static S16
S16FromS32(M_Arena *arena, S32 string)
{
 S16 result = {0};
 
 for(size_t i = 0;
     i < string.len;
     i += 1)
 {
  // NOTE(tbt): UTF16FromCodepoint allocates unaligned so characters guaranteed to be
  //            contiguous in arena - no need to copy anything :)
  S16 s16 = UTF16FromCodepoint(arena, string.buffer[i]);
  if (!result.buffer)
  {
   result.buffer = s16.buffer;
  }
  result.len += s16.len;
 }
 // NOTE(tbt): null terminate to make easy to use with system APIs
 M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
 
 return result;
}

//~NOTE(tbt): string operations

static S8
S8Clone(M_Arena *arena, S8 string)
{
 S8 result;
 result.len = string.len;
 result.buffer = M_ArenaPushAligned(arena, string.len + 1, 1);
 M_Copy(result.buffer, string.buffer, string.len);
 return result;
}

static S8
S8FromFmtV(M_Arena *arena, char *fmt, va_list args)
{
 S8 result;
 
 va_list _args;
 va_copy(_args, args);
 result.len = stbsp_vsnprintf(0, 0, fmt, args);
 result.buffer = M_ArenaPush(arena, result.len + 1);
 stbsp_vsnprintf(result.buffer, result.len + 1, fmt, _args);
 
 return result;
}

static S8
S8FromFmt(M_Arena *arena, char *fmt, ...)
{
 S8 result;
 va_list args;
 va_start(args, fmt);
 result = S8FromFmtV(arena, fmt, args);
 va_end(args);
 return result;
}

static Bool
S8Compare(S8 a, S8 b, S8CompareFlags flags)
{
 Bool is_match;
 
 if(!(flags & S8CompareFlags_RightSideSloppy) && a.len != b.len)
 {
  is_match = False;
 }
 else
 {
  size_t length_to_compare = Min(a.len, b.len);
  
  if(flags & S8CompareFlags_CaseInsensitive)
  {
   is_match = True;
   for(size_t character_index = 0;
       character_index < length_to_compare;
       character_index += 1)
   {
    if(CharLowerFromUpper(a.buffer[character_index]) !=
       CharLowerFromUpper(b.buffer[character_index]))
    {
     is_match = False;
     break;
    }
   }
  }
  else
  {
   is_match = M_Compare(a.buffer, b.buffer, length_to_compare);
  }
 }
 
 return is_match;
}

//~NOTE(tbt): string lists

static void
S8ListPushExplicit(S8List *list, S8Node *string)
{
 if(NULL == list->first)
 {
  string->next = NULL;
  list->first = string;
  list->last = string;
 }
 else
 {
  string->next = list->first;
  list->first = string;
 }
}

static void
S8ListAppendExplicit(S8List *list, S8Node *string)
{
 if(NULL == list->last)
 {
  string->next = NULL;
  list->first = string;
  list->last = string;
 }
 else
 {
  list->last->next = string;
  list->last = string;
 }
}

static void
S8ListPush(M_Arena *arena, S8List *list, S8 string)
{
 S8Node *node = M_ArenaPush(arena, sizeof(*node));
 node->string = S8Clone(arena, string);
 S8ListPushExplicit(list, node);
}

static void
S8ListAppend(M_Arena *arena, S8List *list, S8 string)
{
 S8Node *node = M_ArenaPush(arena, sizeof(*node));
 node->string = S8Clone(arena, string);
 S8ListAppendExplicit(list, node);
}

static S8
S8ListJoin(M_Arena *arena, S8List list)
{
 S8 result = {0};
 for(S8Node *node = list.first;
     NULL != node;
     node = node->next)
 {
  S8 string = S8Clone(arena, node->string);
  if(NULL == result.buffer)
  {
   result.buffer = string.buffer;
  }
  result.len += string.len;
 }
 // NOTE(tbt): null terminate to make easy to use with system APIs
 M_ArenaPushAligned(arena, sizeof(*result.buffer), 1);
 
 return result;
}


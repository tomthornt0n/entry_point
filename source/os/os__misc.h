#define OS_AssertWithMessage(c, msg) Statement( if(!(c)) { OS_ConsoleOutputFmt("%s, line %d: Assertion failed: %s", __FILE__, __LINE__, msg); AssertBreak_(); } )
#define OS_Assert(c) OS_AssertWithMessage(c, "'" Stringify(c) "'")

static void OS_Init(void);
static void OS_Cleanup(void);


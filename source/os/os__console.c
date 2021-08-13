static void
OS_ConsoleOutputFmtV(char *fmt, va_list args)
{
 M_Temp scratch;
 OS_TC_ScratchMem(scratch, NULL, 0)
 {
  S8 string = S8FromFmtV(scratch.arena, fmt, args);
  OS_ConsoleOutputS8(string);
 }
}

static void
OS_ConsoleOutputFmt(char *fmt, ...)
{
 va_list args;
 va_start(args, fmt);
 OS_ConsoleOutputFmtV(fmt, args);
 va_end(args);
}
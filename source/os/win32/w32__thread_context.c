static DWORD w32_tc_id = 0;

static void
OS_TC_Make_(OS_TC *tctx)
{
 return;
}

static OS_TC *
OS_TC_Get(void)
{
 OS_TC *result = TlsGetValue(w32_tc_id);
 return result;
}

static void
OS_TC_Set(OS_TC *tctx)
{
 void *ptr = tctx;
 TlsSetValue(w32_tc_id, ptr);
}
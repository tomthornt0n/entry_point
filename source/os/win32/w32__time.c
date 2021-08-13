
//~NOTE(tbt): internal helpers

static size_t w32_t_perf_counter_ticks_per_second = 0;
static Bool w32_t_is_granular = False;

static T_DateTime
W32_T_DateTimeFromSystemTime(SYSTEMTIME system_time)
{
 T_DateTime result;
 result.year = system_time.wYear;
 result.mon = system_time.wMonth - 1;
 result.day = system_time.wDay - 1;
 result.hour = system_time.wHour;
 result.min = system_time.wMinute;
 result.sec = system_time.wSecond;
 result.msec = system_time.wMilliseconds;
 return result;
}

static SYSTEMTIME
W32_T_SystemTimeFromDateTime(T_DateTime date_time)
{
 SYSTEMTIME result = {0};
 result.wYear = date_time.year;
 result.wMonth = date_time.mon + 1;
 result.wDay = date_time.day + 1;
 result.wHour = date_time.hour;
 result.wMinute = date_time.min;
 result.wSecond = date_time.sec;
 result.wMilliseconds = date_time.msec;
 return result;
}

static T_DenseTime
W32_T_DenseTimeFromFileTime(FILETIME file_time)
{
 T_DenseTime result;
 SYSTEMTIME system_time = {0};
 FileTimeToSystemTime(&file_time, &system_time);
 T_DateTime date_time = W32_T_DateTimeFromSystemTime(system_time);
 result = T_DenseTimeFromDateTime(date_time);
 return result;
}

//~NOTE(tbt): real world time

static T_DateTime
OS_T_UTCGet(void)
{
 T_DateTime result;
 SYSTEMTIME system_time;
 GetSystemTime(&system_time);
 result = W32_T_DateTimeFromSystemTime(system_time);
 return result;
}

static T_DateTime
OS_T_LTCFromUTC(T_DateTime universal_time)
{
 T_DateTime result;
 
 SYSTEMTIME ut_system_time, lt_system_time;
 FILETIME ut_file_time, lt_file_time;
 ut_system_time = W32_T_SystemTimeFromDateTime(universal_time);
 SystemTimeToFileTime(&ut_system_time, &ut_file_time);
 FileTimeToLocalFileTime(&ut_file_time, &lt_file_time);
 FileTimeToSystemTime(&lt_file_time, &lt_system_time);
 result = W32_T_DateTimeFromSystemTime(lt_system_time);
 
 return result;
}

static T_DateTime
OS_T_UTCFromLTC(T_DateTime local_time)
{
 T_DateTime result;
 
 SYSTEMTIME ut_system_time, lt_system_time;
 FILETIME ut_file_time, lt_file_time;
 lt_system_time = W32_T_SystemTimeFromDateTime(local_time);
 SystemTimeToFileTime(&lt_system_time, &lt_file_time);
 LocalFileTimeToFileTime(&lt_file_time, &ut_file_time);
 FileTimeToSystemTime(&ut_file_time, &ut_system_time);
 result = W32_T_DateTimeFromSystemTime(ut_system_time);
 
 return result;
}

//~NOTE(tbt): precision time

static void
OS_T_Sleep(size_t milliseconds)
{
 Sleep(milliseconds);
}

static size_t
OS_T_MicrosecondsGet(void)
{
 size_t result = 0;
 LARGE_INTEGER perf_counter;
 if(QueryPerformanceCounter(&perf_counter))
 {
  result = (perf_counter.QuadPart * 1000000llu) / w32_t_perf_counter_ticks_per_second;
 }
}

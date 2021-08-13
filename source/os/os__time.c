static T_DateTime
OS_T_NowL(void)
{
 T_DateTime result;
 T_DateTime universal_time = OS_T_UTCGet();
 result = OS_T_LTCFromUTC(universal_time);
 return result;
}

static double
OS_T_SecondsGet(void)
{
 size_t microseconds = OS_T_MicrosecondsGet();
 double seconds = (double)microseconds * 0.000001;
 return seconds;
}

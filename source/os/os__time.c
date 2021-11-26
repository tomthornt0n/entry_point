Function T_DateTime
T_LTCGet(void)
{
    T_DateTime result;
    T_DateTime universal_time = T_UTCGet();
    result = T_LTCFromUTC(universal_time);
    return result;
}

Function double
T_SecondsGet(void)
{
    size_t microseconds = T_MicrosecondsGet();
    double seconds = (double)microseconds * 0.000001;
    return seconds;
}

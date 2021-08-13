
//~NOTE(tbt): real world time

static T_DateTime OS_T_UTCGet(void);                          // NOTE(tbt): current date-time in universal time coordinates
static T_DateTime OS_T_LTCGet(void);                          // NOTE(tbt): current date-time in local time coordinates
static T_DateTime OS_T_LTCFromUTC(T_DateTime universal_time); // NOTE(tbt): convert from universal time coordinates to local time coordinates
static T_DateTime OS_T_UTCFromLTC(T_DateTime local_time);     // NOTE(tbt): convert from local time coordinates to universal time coordinates

//~NOTE(tbt): precision time

static void OS_T_Sleep(size_t milliseconds);

static size_t OS_T_MicrosecondsGet(void); // NOTE(tbt): query the performance counter to get the current time in microseconds
static double OS_T_SecondsGet(void);      // NOTE(tbt): query the performance counter and convert to seconds

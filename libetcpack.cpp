#ifdef __ANDROID__
#include <time.h>
#include <sys/timeb.h>
struct timeb_
{
    time_t time; //The seconds portion of the current time.
    unsigned short millitm; //  The milliseconds portion of the current time.
    short timezone; // The local timezone in minutes west of Greenwich.
    short dstflag; //  TRUE if Daylight Savings Time is in effect.
};
int ftime_(struct timeb_ *tb)
{
    struct timeval  tv;
    struct timezone tz;

    if (gettimeofday(&tv, &tz) < 0)
        return -1;

    tb->time = tv.tv_sec;
    tb->millitm = (tv.tv_usec + 500) / 1000;

    if (tb->millitm == 1000) {
        ++tb->time;
        tb->millitm = 0;
    }
    tb->timezone = tz.tz_minuteswest;
    tb->dstflag = tz.tz_dsttime;

    return 0;
}
#define _timeb timeb_
#define _ftime ftime_
#endif

#include "source/etcpack.cxx"
#define table59T table59T_dec
#define table58H table58H_dec
#define compressParams compressParams_dec
#define unscramble unscramble_dec
#define PATTERN_H PATTERN_H_dec
#define PATTERN_T PATTERN_T_dec
#include "source/etcdec.cxx"
#include "source/image.cxx"

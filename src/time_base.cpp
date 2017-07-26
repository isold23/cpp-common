#include "time_base.h"

uint64_t CTimeBase::get_current_time()
{
    struct timeb loTimeb;
    memset(&loTimeb, 0 , sizeof(timeb));
    ftime(&loTimeb);
    return ((uint64_t)loTimeb.time * 1000) + loTimeb.millitm;
}

uint64_t CTimeBase::escape_time(uint64_t last_time)
{
    return 0;
}

std::string CTimeBase::get_now()
{
    std::string strCurrentTime = "";
    struct timeb loSystemTime;
    ftime(&loSystemTime);
    struct tm* lptm = localtime(&loSystemTime.time);
    stringstream lstrTimeNow;
    lstrTimeNow.fill('0');
    lstrTimeNow << setw(4) << 1900 + lptm->tm_year << "-" <<
                setw(2) << 1 + lptm->tm_mon << "-" <<
                setw(2) << lptm->tm_mday << " " <<
                setw(2) << lptm->tm_hour << ":" <<
                setw(2) << lptm->tm_min << ":" <<
                setw(2) << lptm->tm_sec << ":" <<
                setw(3) << loSystemTime.millitm << ends;
    strCurrentTime = lstrTimeNow.str();
    return strCurrentTime;
}

std::string CTimeBase::conver_timestamp(const uint32_t timestamp)
{
    std::string strTime = "";
    
    if(timestamp == 0) {
        strTime = "0000-00-00 00:00:00";
    } else {
        time_t tt = timestamp;
        struct tm* lptm;
        lptm = localtime(&tt);
        stringstream lstrTimeNow;
        lstrTimeNow.fill('0');
        lstrTimeNow << setw(4) << 1900 + lptm->tm_year << "-" <<
                    setw(2) << 1 + lptm->tm_mon << "-" <<
                    setw(2) << lptm->tm_mday << " " <<
                    setw(2) << lptm->tm_hour << ":" <<
                    setw(2) << lptm->tm_min << ":" <<
                    setw(2) << lptm->tm_sec;
        strTime = lstrTimeNow.str();
    }
    
    return strTime;
}

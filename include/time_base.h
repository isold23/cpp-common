#ifndef _TIME_H_
#define _TIME_H_

#include <string>
#include <iostream>
#include <iomanip>

#include "include.h"

inline uint64_t timediff_us(const struct timeval& s, const struct timeval& e)
{
    return (e.tv_sec - s.tv_sec) * 1000000UL + (e.tv_usec - s.tv_usec);
}
          
#define START_COUNT_TIME(proc_name) \
    timeval time_start_##proc_name = {0, 0}; \
    timeval time_end_##proc_name = {0, 0}; \
    gettimeofday(&time_start_##proc_name, NULL)
               
#define END_COUNT_TIME(proc_name) \
    gettimeofday(&time_end_##proc_name, NULL); \
    uint64_t time_##proc_name = timediff_us(time_start_##proc_name, time_end_##proc_name);\
    LOG_DEBUG("time cost[%s:%luus]", #proc_name, time_##proc_name)

class CTimeBase
{
public:
    static uint64_t get_current_time(void);
    static uint64_t escape_time(uint64_t last_time);
    
    //2009-12-02 12:02:30:234
    static std::string get_now(void);
    static std::string conver_timestamp(uint32_t timestamp);
};

#endif //_TIME_H_





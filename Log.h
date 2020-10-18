//
// Created by zach on 10/18/2020.
//

#ifndef BGP_LOG_H
#define BGP_LOG_H

#include <string>
#include <algorithm>
#include <date/date.h>
#include <iostream>
#include <mutex>

static std::mutex _logging_mutex;

static __forceinline void PrintLogMessage(std::string severity, std::string message) {
    _logging_mutex.lock();
    std::transform(severity.begin(), severity.end(), severity.begin(), ::toupper);
#ifdef NDEBUG
    if (severity == "DEBUG") {
        return;
    }
#endif
    auto tp = std::chrono::system_clock::now();
//    auto tp = std::chrono::time_point<std::chrono::system_clock>{};
    auto dp = date::floor<date::days>(tp);
    auto ymd = date::year_month_day{dp};
    auto time = date::make_time(tp-dp);
    std::clog << ymd << "T" << time << ": " << severity << ": " << message << std::endl;
    _logging_mutex.unlock();
}

#endif //BGP_LOG_H

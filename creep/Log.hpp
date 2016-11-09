#ifndef CREEP_LOG_HPP
#define CREEP_LOG_HPP

#ifndef DISABLE_LOGS
#include <iostream>
#define LOG std::cerr
#else
struct DummyLogger {};

template<typename T>
const DummyLogger& operator<<(const DummyLogger& l, const T&) {
    return l;
}

#define LOG DummyLogger{}
#endif

#endif // CREEP_LOG_HPP

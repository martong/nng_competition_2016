#ifndef CREEP_LOG_HPP
#define CREEP_LOG_HPP

struct DummyLogger {};

template<typename T>
const DummyLogger& operator<<(const DummyLogger& l, const T&) {
    return l;
}

#ifndef DISABLE_LOGS
#define LOG std::cerr
#else
#define LOG DummyLogger{}
#endif

#endif // CREEP_LOG_HPP

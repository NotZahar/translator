#pragma once

namespace ts {
    /*!
        \brief Non-copyable and non-movable
    */
    #define TS_NCM(T) T(const T&) = delete; \
        T& operator=(const T&) = delete;    \
        T(T&&) = delete;                    \
        T& operator=(T&&) = delete;
}
#pragma once
#include <cstdio>
#include <cstring>

inline const char* center_text(const char* text, int width, char* buf)
{
    const int len = static_cast<int>(std::strlen(text));

    if (len >= width)
    {
        std::memcpy(buf, text, width);
        buf[width] = '\0';
        return buf;
    }

    const int padding = width - len;
    const int left  = padding / 2;
    const int right = padding - left;

    std::memset(buf, ' ', width);
    std::memcpy(buf + left, text, len);
    buf[width] = '\0';

    return buf;
}


inline void log_fsm(State state, Event ev, State next)
{
#ifdef DEBUG
    constexpr int STATE_W = 14;
    constexpr int EVENT_W = 22;

    char ev_buf[EVENT_W + 1];

    std::fprintf(stderr,
        "[FSM] %-*s --(%s)--> %-*s\n",
        STATE_W, to_string(state),
        center_text(to_string(ev), EVENT_W, ev_buf),
        STATE_W, to_string(next));
#endif
}

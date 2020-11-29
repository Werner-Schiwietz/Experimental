#pragma once

#include "Signal.h"

#include <string>

struct Signaled1
{
    using this_t = Signaled1;
    std::string string{ "hallo" };

    WS::Connection_Guard event_notify;
    void notified( int & value )
    {
        ++value;
        //traceln(__FUNCTION__ " value=", value, " text:", this->string );
    }
    this_t(){}
    this_t( WS::Signal<void(int&)> & event );
};

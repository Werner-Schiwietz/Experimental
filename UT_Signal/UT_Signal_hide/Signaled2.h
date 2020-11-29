#pragma once

#include "Signal.h"

#include <string>

struct Signaled2
{
    std::string string{ "hallo2" };

    WS::Connection_Guard event_notify;
    void notified( int & value )
    {
        ++value;
        //traceln(__FUNCTION__ " value=", value, " text:", this->string );
    }
    Signaled2(){}
    Signaled2( WS::Signal<void(int&)> & event );
};

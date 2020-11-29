#include "pch.h"
#include "Signaled2.h"

#include "signal_imp.h"

Signaled2::Signaled2( WS::Signal<void(int&)> & event )
    : event_notify( event.connect(std::bind(&Signaled2::notified, this, std::placeholders::_1)) )
{}





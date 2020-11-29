#include "pch.h"
#include "Signaled1.h"

#include "signal_imp.h"

Signaled1::Signaled1( WS::Signal<void(int&)> & event )
    : event_notify( event.connect(std::bind(&Signaled1::notified, this, std::placeholders::_1)) )
{}





#pragma once

#include "Signal.h"

#pragma warning(push)
#pragma warning(disable:4996)
#pragma warning(disable:4996)
#include "boost/signals2.hpp"
#pragma warning(pop)

namespace WS
{
    template<typename return_t, typename ... Args>
    struct Signal<return_t (Args...)> :: MyData
    {
        boost::signals2::signal<void(Args...)> signalhandler;
    };

    template<typename return_t, typename ... Args> Signal<return_t (Args...)>::~Signal()
    {
    }

    template<typename return_t, typename ... Args> Signal<return_t (Args...)>::Signal()
        : mydata(new MyData{})
    {
    }

    struct Connection_Guard::Data
    {
        boost::signals2::connection connection;
    };

    template<typename return_t, typename ... Args>
    Connection_Guard Signal<return_t (Args...)>::connect( std::function<return_t(Args...)> callback )
    {
        auto connection = this->mydata->signalhandler.connect( callback );
        return {Connection_Guard::Data{std::move(connection)}};
    }

    template<typename return_t, typename ... Args>
    void Signal<return_t (Args...)>::operator()(Args ... args)
    {
        this->mydata->signalhandler( std::forward<Args>(args)... );
    }
}


#pragma once

#ifndef EXT_CLASS
#   define EXT_CLASS
#endif

#include <memory>
#include <functional>

namespace WS
{
    //wenn der connection_guard zerstört wird, wird die verbindung zum signal gelöst
    struct EXT_CLASS Connection_Guard
    {
    private:
        struct Data;
        //std::unique_ptr<Data> mydata{nullptr};
        Data* mydata{nullptr};
    public:
        ~Connection_Guard();
        Connection_Guard(){}
        Connection_Guard( Data && connection );
        Connection_Guard( Connection_Guard const &) = delete;
        Connection_Guard( Connection_Guard &&);

        Connection_Guard& operator=( Connection_Guard const &) = delete;
        Connection_Guard& operator=( Connection_Guard &&);

        void swap( Connection_Guard & );
        template<typename signatur> friend class Signal;
    };

    template<typename signatur> class Signal;
    template<typename return_t, typename ... Args>class Signal<return_t (Args...)>
    {
        struct MyData;//enthaelt boost::signals2::signal
        std::unique_ptr<MyData> mydata;

    public:
        EXT_CLASS Signal();
        EXT_CLASS ~Signal();

        //möchte vom event benachrichtigt werden
        EXT_CLASS Connection_Guard connect( std::function<return_t(Args...)> );

        //löst event aus und benachrichtigt alle registrierten
        EXT_CLASS void operator()(Args...);//TODO das mit dem return-wert
    };
}


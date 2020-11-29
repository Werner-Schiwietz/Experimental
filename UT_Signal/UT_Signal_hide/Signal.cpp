#include "pch.h"

#include "signal_imp.h"

namespace WS
{
    Connection_Guard::~Connection_Guard()
    {
        if( this->mydata )
        {
            this->mydata->connection.disconnect();
            delete this->mydata;
            this->mydata = nullptr;
        }
    }

    Connection_Guard::Connection_Guard( Connection_Guard::Data && connection )
        : mydata( new Connection_Guard::Data{std::move(connection)} )
    {
    }

    Connection_Guard::Connection_Guard( Connection_Guard && r)
    {
        swap(r);
    }

    Connection_Guard& Connection_Guard::operator=( Connection_Guard &&r)
    {
        Connection_Guard {std::move(r)}.swap(*this);
        return *this;
    }

    void Connection_Guard::swap( Connection_Guard & r)
    {
        std::swap(this->mydata, r.mydata);
    }
}

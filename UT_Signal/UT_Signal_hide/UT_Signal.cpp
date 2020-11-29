#include "pch.h"
#include "CppUnitTest.h"

#include "Signal.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


#include <atlstr.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//#include "..\BASIS\UT_TraceToLogger.h"
//namespace
//{
//    TestLogger TraceOutputInsOutputTestsFenster;
//}
//#define USE_AUSGABEKANAL &TraceOutputInsOutputTestsFenster
//#include "..\BASIS\trace.h"

#include "signal_imp.h"

namespace UTSignal
{
    TEST_CLASS(UTSignal)
    {
        TEST_METHOD(UT_lambda)
        {
            auto triggered_throw = []( int value )->int//rückgabewert sinnlos
            {
                //traceln(__FUNCTION__ "value=", value );
                throw value;
            };

            auto triggered = []( int value )->int//rückgabewert sinnlos
            {
                //traceln(__FUNCTION__ "value=", value );
                return value;
            };

            WS::Signal<int(int)> event;

            event(1);//geht ins leere
            {
                event.connect(triggered_throw);
                event(2);//geht auch ins leere
            }

            {
                auto signal_connection = event.connect(triggered);
                event(5);//geht nicht ins leere
            }
            event(3);//geht auch ins leere
            {
                //fehlerfall, connection lebt länger als callback
                auto connection = [&]()
                {
                    auto local = []( int value )->int
                    {
                        //traceln(__FUNCTION__ "value=", value );
                        return value;
                    };
                    return event.connect(local);
                }();
                event(6);//wohin sollte der aufruf gehen???
            }
        }

        TEST_METHOD(UT_method)
        {
            WS::Signal<int(int&)> event;
            struct A
            {
                wchar_t const * string = L"hallo";
                WS::Connection_Guard event_notify;
                int notified( int & value )
                {
                    ++value;
                    //traceln(__FUNCTION__ " value=", value, " text:", this->string );
                    return value;
                }
                A(){}
                A( WS::Signal<int(int&)> & event )
                    : event_notify( event.connect(std::bind(&A::notified, this, std::placeholders::_1)) )
                {
                }
            };

            {
                A aa{};

                std::function<int(int&)> fn = std::bind(&A::notified, &aa, std::placeholders::_1);
                aa.event_notify = event.connect(fn);

                A a( event );

                int i=0;
                event(i);//zwei funktionen event signaliisert
                Assert::IsTrue(i==2);
            }

            int i=0;
            event(i);//will niemand mehr wissen
            Assert::IsTrue(i==0);
        }
    };
}


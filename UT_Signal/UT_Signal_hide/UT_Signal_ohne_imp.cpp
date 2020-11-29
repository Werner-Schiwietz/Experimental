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

#include "Signaled1.h"
#include "Signaled2.h"

namespace UTSignal
{
    TEST_CLASS(UTSignalOhneImp)
    {
        TEST_METHOD(UT_method)
        {
            //boost::signals2::signal nicht bekannt, weil nicht direkt includiert. das war das ziel dieser aktion
            //boost::signals2::signal<void(Args...)> signalhandler;

            WS::Signal<void(int&)> event;
            {
                Signaled1 aa{};

                std::function<void(int&)> fn = std::bind(&Signaled1::notified, &aa, std::placeholders::_1);
                aa.event_notify = event.connect(fn);

                Signaled2 a( event );

                int i=0;
                event(i);//zwei funktionen event signalisert
                Assert::IsTrue(i==2);
            }

            int i=0;
            event.operator()(i);//will niemand mehr wissen
            Assert::IsTrue(i==0);
        }
    };
}


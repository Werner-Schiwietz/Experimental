#include "pch.h"
#include "CppUnitTest.h"

#include "Signal.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


#include <atlstr.h>
#include <functional>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//#include "..\BASIS\UT_TraceToLogger.h"
//namespace
//{
//    TestLogger TraceOutputInsOutputTestsFenster;
//}
//#define USE_AUSGABEKANAL &TraceOutputInsOutputTestsFenster
//#include "..\BASIS\trace.h"


bool foo1(int a, int b)
{
    return a==b;
}
struct foo2
{
    bool operator()(int a,int b)
    {
        return a==b;
    }
};
auto foo3 = [](int a, int b)->bool{return a==b;};
auto foo4 = [v=5](int a, int b)->bool{return a==b;};
std::function<bool(int,int)> foo;


template<typename fn_type, typename signatur>struct Calling_signatur;
template<typename fn_type, typename return_type, typename ... parameter_types>struct Calling_signatur<fn_type,return_type(parameter_types...)>
{
    using fn_t              = fn_type;
    using return_t          = return_type;
    //typedef parameter_types   parameter_t;
    //using parameter_t=parameter_types;
    //using parameter_t...=parameter_types;
    //using parameter_t...=parameter_types...;
    //using parameter_t=parameter_types...;
    fn_t fn;

    Calling_signatur( fn_t fn ) : fn(fn)
    {
        //fn=fn;geht mit lambda nicht
    }
    return_t operator()( parameter_types... args)
    {
        return fn(std::forward<parameter_types>(args)...);
    }
};
template<typename signatur, typename fn_type> struct Calling : Calling_signatur<fn_type,signatur>
{
    Calling( fn_type fn ) : Calling_signatur(fn){}
};
template<typename signatur, typename fn_type> Calling<typename signatur, typename fn_type> make_Calling( fn_type fn )
{
    return Calling<signatur,fn_type>{std::move(fn)};
}



namespace UTSignal
{
    TEST_CLASS(UT_Funktionen)
    {
        TEST_METHOD(UT_Test1)
        {
            bool (*f)(int,int) = foo1;
            Assert::IsTrue(f(5,5));
            Assert::IsFalse(f(5,6));
            auto callback = Calling<bool(int,int),decltype(&foo1)>{foo1};
            Assert::IsTrue(callback(5,5));
            Assert::IsFalse(callback(5,6));
            auto callback2 = make_Calling<bool(int,int)>(foo1);
            Assert::IsTrue(callback2(5,5));
            Assert::IsFalse(callback2(5,6));
        }
        TEST_METHOD(UT_Test2)
        {
            auto callback = make_Calling<bool(int,int)>(foo2{});
            Assert::IsTrue(callback(5,5));
            Assert::IsFalse(callback(5,6));
        }
        TEST_METHOD(UT_Test3)
        {
            auto callback = Calling<bool(int,int),decltype(foo3)>{foo3};
            Assert::IsTrue(callback(5,5));
            Assert::IsFalse(callback(5,6));
        }
        TEST_METHOD(UT_Test4)
        {
            auto callback = make_Calling<bool(int,int)>(foo4);
            Assert::IsTrue(callback(5,5));
            Assert::IsFalse(callback(5,6));
        }
        TEST_METHOD(UT_Test5)
        {
            std::function<bool(int,int)> fn;

            {
                auto callback = Calling<bool(int,int),std::function<bool(int,int)>>{std::function<bool(int,int)>{&foo1}};
                Assert::IsTrue(callback(5,5));
                Assert::IsFalse(callback(5,6));
            }
            {
                auto callback = Calling<bool(int,int),std::function<bool(int,int)>>{std::function<bool(int,int)>{foo2{}}};
                Assert::IsTrue(callback(5,5));
                Assert::IsFalse(callback(5,6));
            }
            {
                auto callback = Calling<bool(int,int),std::function<bool(int,int)>>{std::function<bool(int,int)>{foo3}};
                Assert::IsTrue(callback(5,5));
                Assert::IsFalse(callback(5,6));
            }
            {
                auto callback = Calling<bool(int,int),std::function<bool(int,int)>>{std::function<bool(int,int)>{foo4}};
                Assert::IsTrue(callback(5,5));
                Assert::IsFalse(callback(5,6));
            }
        }
        TEST_METHOD(UT_Test6)
        {
            std::function<bool(int,int)> fn;

            //auto callback = Calling<std::function<bool(int,int)>,bool(int,int)>{fn};//fn wird kopiert, also nachträglich eingetragene fn haben keine auswirkung
            auto callback = Calling<bool(int,int),std::function<bool(int,int)> const &>{fn};//fn als ref sollte es gehen
            fn = &foo1;
            Assert::IsTrue(callback(5,5));
            Assert::IsFalse(callback(5,6));
            fn = foo2{};
            Assert::IsTrue(callback(5,5));
            Assert::IsFalse(callback(5,6));
            fn = foo3;
            Assert::IsTrue(callback(5,5));
            Assert::IsFalse(callback(5,6));
            fn = foo4;
            Assert::IsTrue(callback(5,5));
            Assert::IsFalse(callback(5,6));
        }

    };
}


#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <cassert>
#include <string>

#include <atlstr.h>
#include <CRTDBG.H>

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)


#ifdef DEBUG
//#	define ASSERT(x) _ASSERTE(x)//no STRINGIFY2(x)"run"-mode
#	define ASSERT(x) _ASSERT_EXPR(((bool)(x))==true,_CRT_WIDE(#x))//no message in "run"-mode
#	define VERIFY(x) ASSERT(x)
#else
#	define ASSERT(x) ((void)0)
#	define VERIFY(x) (void)(x)
#endif
#define VERIFY_DOIFFALSE(x,action) \
	{	if( static_cast<bool>(x)==false ) \
		{	_ASSERT_EXPR(false,_CRT_WIDE(#x)); \
			action \
	} 	}
#define VERIFY_RETURN(x,ret_value) VERIFY_DOIFFALSE(x,return ret_value;)

//static_assert( std::is_same_v<wchar_t,CString::XCHAR> );
//static_assert( std::is_same_v<char,CString::XCHAR> );

std::string getHallo(){return "hallo";}
std::string getEmtpyString(){return {};}

template<typename fn_t>
int test1(fn_t fn)
{
	std::string v;
	bool success;
	VERIFY(success=(bool)((v=fn()).empty()));
	if( success )
	{
		return 1;
	}
	return 0;
}
template<typename fn_t>
int test2(fn_t fn)
{
	VERIFY_RETURN( fn().empty(), 0 );
	return 1;
}

template<typename T>
bool test3( T x )
{
	VERIFY_RETURN(x,false);
	return true;
}

namespace UTAssert
{
	TEST_CLASS(UTAssert)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			Assert::IsTrue(0==test1(getHallo) );
			Assert::IsTrue(1==test1(getEmtpyString) );
		}
		TEST_METHOD(TestMethod2)
		{
			Assert::IsTrue(0==test2(getHallo) );
			Assert::IsTrue(1==test2(getEmtpyString) );
		}
		TEST_METHOD(TestMethod3)
		{
			std::string str{"hallo"};
			std::string const * ptr = &str;

			Assert::IsTrue(test3(ptr) );
			ptr = {};
			Assert::IsFalse(test3(ptr) );
		}
	};
}

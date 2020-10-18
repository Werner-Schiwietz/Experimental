#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

struct doubleE10
{
	using mantisse_t = __int32;
	using exponent_t = __int8;

	mantisse_t mantisse : 24 = 0;
	exponent_t exponent	: 8	= 0;
};

namespace UTAllerei
{
	TEST_CLASS(UT_MyDoubleE10_natvis)
	{

	public:
		
		TEST_METHOD(Show_doubleE10)
		{
			auto v = doubleE10{100,2};
			auto x = v.mantisse / pow(10,v.exponent);x;

			v = doubleE10{1000,3};
			v = doubleE10{10,2};
			v = doubleE10{1,2};
			v = doubleE10{1,-2};

			v=v;
		}
	};
}

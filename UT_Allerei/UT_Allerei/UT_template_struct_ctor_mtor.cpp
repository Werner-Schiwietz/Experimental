#include "pch.h"
#include "CppUnitTest.h"

#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

template <typename T> struct AA
{
	using type=T;
	static constexpr bool noMtor = std::is_same_v<T const &,T &&>;
};


template <typename T,bool no_mtor=std::is_same_v<T const &,T &&>> struct A
{
	A(){}
	A(T const & v)
	{v;}
	A(T && v)
	{v;}
};
template <typename T> struct A<T,true>
{
	A(){}
	A(T const & v)
	{v;}
};
namespace UT_template_ctor_mtor
{
	TEST_CLASS(UT_)
	{
	public:
		TEST_METHOD(UT_ctor_and_mtor)
		{
			static_assert(AA<int>::noMtor == false );
			static_assert(AA<int &>::noMtor == true );
			static_assert(AA<int const &>::noMtor == true );
		}
		TEST_METHOD(UT_einfacher_typ)
		{
			int v=5;
			A<int>{v};
		}
		TEST_METHOD(UT_einfacher_typ_ref)
		{
			int v=5;
			A<int &>{v};
		}
		TEST_METHOD(UT_einfacher_typ_const_ref)
		{
			int v=5;
			A<int const &>{v};
		}
	};
}

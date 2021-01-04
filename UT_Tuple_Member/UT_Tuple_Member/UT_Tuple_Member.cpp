#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <tuple>

struct Member
{
	static size_t constexpr n0 = 0;
	static size_t constexpr n1 = 1;
	std::tuple<int,std::string> member;

	Member(){};
	Member(int const & n0,std::string const & n1):member(n0,n1){}
	template<size_t name> auto& get() & {return std::get<name>(member);}
	template<size_t name> auto const & get() const & {return std::get<name>(member);}
};

namespace UTTupleMember
{
	TEST_CLASS(UTTupleMember)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			struct A : Member
			{
				using Member::Member;
			};

			auto a = A{5,"hallo"};

			Assert::IsTrue( a.get<A::n0>() == 5 );
			Assert::IsTrue( a.get<A::n1>() == "hallo" );
			//Assert::IsTrue( a.get<2>() == "hallo" );
		}
	};
}

#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace WS
{
	template<typename...T> struct is_shared_ptr : std::false_type{};
	template<typename...Ts> struct is_shared_ptr<std::shared_ptr<Ts...>> : std::true_type{};
	template<typename T> static auto constexpr is_shared_ptr_v = is_shared_ptr<T>::value;
}

#include <string>
#include <memory>


namespace UT_template_ctor_mtor
{
	TEST_CLASS(UT_)
	{
	public:
		TEST_METHOD(UT_is_shared_ptr)
		{
			auto deleter = [](int*& p)
			{
				delete p;
				p=nullptr;
			};
			std::shared_ptr<int> p4 {new int{4}};
			std::shared_ptr<int> p5 {new int{5}, deleter, std::allocator<int>()};
			std::unique_ptr<int,decltype(deleter)> p6 {new int{6}, deleter};
			std::unique_ptr<int,decltype(deleter)> p7 {new int{7}};


			Assert::IsTrue( WS::is_shared_ptr<std::shared_ptr<int>>::value );
			Assert::IsFalse( WS::is_shared_ptr<std::unique_ptr<int>>::value );
			Assert::IsFalse( WS::is_shared_ptr<int>::value );
			Assert::IsTrue( WS::is_shared_ptr<decltype(p5)>::value );
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(p5)> );
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(p4)> );
			Assert::IsFalse( WS::is_shared_ptr<decltype(p7)>::value );
			Assert::IsFalse( WS::is_shared_ptr_v<decltype(p6)> );
		}
	};
}

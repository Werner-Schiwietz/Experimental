#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
#include <vector>

template<typename T>struct prevnext
{
	T * prev = nullptr;
	T * next = nullptr;
};


struct A : prevnext<A>{ int v=1;};
struct AA : std::vector<AA>
{
};


template<typename T> struct Item
{
	int data;
};

enum Foo { kBar };

namespace UTAllerei
{
	TEST_CLASS(Ableitung_natvis)
	{

	public:
		
		TEST_METHOD(Show_ableitung)
		{
			A a1,a2,a3;
			AA aa;
			a1.next = &a2;
			a2.next = &a3;

			struct B : prevnext<B>{ int v=2; };
			struct BB : std::vector<BB>
			{
			};
			B b1,b2,b3;
			BB bb;
			b1.next = &b2;
			b2.next = &b3;
			int i = 1;
			i = 2;
		}
		TEST_METHOD(main)
		{
			enum Foo { kBar };
			Item<Foo> item = { (uintptr_t)Foo::kBar };
			Item<::Foo> item2 = { ::kBar };

			0==0;
		}

	};
}

#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "signal.h"


namespace _1
{
	template<typename T>struct Combiner
	{
		std::optional<T> erg;
		Combiner& operator<<( T v) & { erg = v; return *this;}
		Combiner operator<<( T v) && { erg = v; return *this;}
	};
	//template<>struct Combiner<void>
	//{
	//	Combiner& operator<< ( ) & { return *this;}
	//	Combiner operator<< ( ) && { return *this;}
	//};
}
namespace _2
{
	template<typename T>struct Combiner
	{
		std::optional<T> erg;
		Combiner& operator()( T v) &  { erg = v; return *this;}
		Combiner  operator()( T v) && { erg = v; return *this;}
	};
	template<>struct Combiner<void>
	{
		Combiner& operator( )( void ) &  { return *this;}
		Combiner  operator( )( void ) && { return *this;}
	};
}
namespace _3
{
	template<typename T>struct Combiner
	{
		std::optional<T> erg;
		Combiner& doit( T v) &  { erg = v; return *this;}
		Combiner  doit( T v) && { erg = v; return *this;}
	};
	template<>struct Combiner<void>
	{
			Combiner& doit( ) &  { return *this;}
			Combiner  doit( ) && { return *this;}
	};
}
//namespace _4
//{
//	template<typename signatur>struct Combiner : WS::template_signatur<signatur>
//	{
//		std::optional<T> erg;
//		Combiner& doit( ) &  { erg = v; return *this;}
//		Combiner  doit( T v) && { erg = v; return *this;}
//	};
//	template<>struct Combiner<void>
//	{
//		Combiner& doit( ) &  { return *this;}
//		Combiner  doit( ) && { return *this;}
//	};
//}

namespace
{
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
}

namespace UTSignal
{
	TEST_CLASS(UT_Helper)
	{
	public:
		template<typename signature> struct A : WS::template_signatur<signature>
		{
		};
		TEST_METHOD(UT_)
		{
			A<int(char const*)> a1;a1;
			A<int(char const*,short)> a2;a2;
			std::function<A<int(char const*)>::signatur_t> xx;xx;
			A<int(char const*)>::return_type r;r;
			using first_t=decltype(A<int(char const*,short)>::tuple_t::_Myfirst);
			//decltype(A<int()>::tuple_t::_Myfirst);

		}
		TEST_METHOD(UT_test_using_void_retvalue)
		{
			auto fnint=[](){return 1;};
			auto fnvoid=[](){};
			auto fnx = [](auto x){};
			//auto x = fn();
			fnx (fnint());
			//fnx (fnvoid());//geht nicht
		}
		TEST_METHOD(UT_Combine_int_1)
		{
			{
				auto fn=[](int & i){return i++;};
				int i=1;
				Assert::IsTrue( 4 == (_1::Combiner<int>{}  << fn(i) << fn(i) << fn(i)).erg.value() );
			}
		}
		//TEST_METHOD(UT_Combine_void_1)
		//{
		//	{
		//		auto fn=[](int & i){i++;};
		//		int i=1;
		//		_1::Combiner<void>{}  << fn(i) << fn(i) << fn(i);
		//	}
		//}
		TEST_METHOD(UT_Combine_int_2)
		{
			{
				auto fn=[](int & i){return i++;};
				int i=1;
				Assert::IsTrue( 4 == (_2::Combiner<int>{}( fn(i) )( fn(i) )( fn(i) )).erg.value() );
			}
		}
		//TEST_METHOD(UT_Combine_void_2)
		//{
		//	{
		//		auto fn=[](int & i){i++;};
		//		int i=1;
		//		Assert::IsTrue( 4 == (_2::Combiner<void>{}( fn(i) )( fn(i) )( fn(i) )).erg.value() );
		//	}
		//}
		TEST_METHOD(UT_Combine_int_3)
		{
			{
				auto fn=[](int & i){return i++;};
				int i=1;
				Assert::IsTrue( 4 == (_3::Combiner<int>{}.doit( fn(i) ).doit( fn(i) ).doit( fn(i) )).erg.value() );
			}
		}
		//TEST_METHOD(UT_Combine_void_3)
		//{
		//	{
		//		auto fn=[](int & i){i++;};
		//		int i=1;
		//		Assert::IsTrue( 4 == (_3::Combiner<void>{}.doit( fn(i) ).doit( fn(i) ).doit( fn(i) )).erg.value() );
		//	}
		//}
	};
	TEST_CLASS(UTSignal)
	{
	public:

		TEST_METHOD(UT_combiner_last)
		{
			using T=int;
			{
				WS::combiner_last<T,T{6}> combiner{};
				Assert::IsTrue( combiner.init_val == T{6} );
				Assert::IsTrue( combiner() == T{6} );
			}
			WS::combiner_last<T> combiner{};
			Assert::IsTrue( WS::combiner_last<T>::init_val == T{} );

			static_assert(std::is_same_v<void,T> == false );
			std::optional<T> value;
			for( auto i : {1,2,4,5} )
			{
				value = combiner(i);
				Assert::IsTrue( value.value()==i );
			}
			Assert::IsTrue( value.value()==5);
		}
		TEST_METHOD(UT_combiner_last_void)
		{
			using T=void;
			WS::combiner_last<T> combiner{};
		}
		TEST_METHOD(UT_combiner_and)
		{
			Assert::IsTrue(  WS::combiner_and<bool>{}(true)(true)(true)(true).value.value() );
			Assert::IsFalse( WS::combiner_and<bool>{}(true)(true)(true)(false).value.value() );
			Assert::IsFalse( WS::combiner_and<bool>{}(false)(true)(true)(true).value.value() );
			Assert::IsFalse( WS::combiner_and<bool>{}.value.has_value() );
		}
		TEST_METHOD(UT_combiner_or)
		{
			Assert::IsTrue(  WS::combiner_or<bool>{}(true)(true)(true)(true).value.value() );
			Assert::IsTrue( WS::combiner_or<bool>{}(true)(true)(true)(false).value.value() );
			Assert::IsTrue( WS::combiner_or<bool>{}(false)(true)(true)(true).value.value() );
			Assert::IsFalse( WS::combiner_or<bool>{}(false)(false)(false)(false).value.value() );
			Assert::IsFalse( WS::combiner_or<bool>{}.value.has_value() );
		}

		TEST_METHOD(UT_Combiner_Last_int)
		{
			using T=int;
			WS::CombinerLastvalue<T> combiner{};

			static_assert(std::is_same_v<void,T> == false );
			std::optional<T> value;
			auto fn =[](T & v)->T{return v;};
			int i{0};
			for( ; i<4; ++i )
				value = combiner(value,fn(++i));
			Assert::IsTrue( value.value()==i);
		}
		TEST_METHOD(UT_Combiner_Last_void)
		{
			using T=void;
			WS::CombinerLastvalue<T> combiner{};

			static_assert(std::is_same_v<void,T> );
			static_assert(std::is_same_v<void,T> == true );
			auto fn =[](int & v)->void{};
			int i{1};
			for( ; i<4; ++i )
				fn(++i);
		}
		TEST_METHOD(UT_Signal_cominer_and)
		{
			WS::Signal<bool(int,int),WS::combiner_and<bool>> sig;
			Assert::IsFalse( sig(5,5).has_value() );
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				Assert::IsTrue( sig(5,5).has_value() );
				Assert::IsTrue(  sig(5,5).value() );
				Assert::IsFalse( sig(5,6).value() );
			}
			Assert::IsFalse( sig(5,5).has_value() );
		}
		TEST_METHOD(UT_Signal_cominer_or)
		{
			WS::Signal<bool(int,int),WS::combiner_or<bool>> sig;
			Assert::IsFalse( sig(5,5).has_value() );
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				Assert::IsTrue( sig(5,5).has_value() );
				Assert::IsTrue(  sig(5,5).value() );
				Assert::IsFalse( sig(5,6).value() );
			}
			Assert::IsFalse( sig(5,5).has_value() );
		}
		TEST_METHOD(UT_Signal_standard_cominerlast)
		{
			WS::Signal<bool(int,int)> sig;
			sig(5,5);
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				Assert::IsTrue(  sig(5,5) );
				Assert::IsFalse( sig(5,6) );
			}
			sig(5,5);
		}
		TEST_METHOD(UT_Signal_standard_void)
		{
			WS::Signal<void(int,int)> sig;
			sig(5,5);
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				sig(5,5);
				sig(5,6);
			}
			sig(5,5);
		}
		TEST_METHOD(UT_Signal_standard_all)
		{
			WS::Signal<bool(int,int),WS::combiner_all<bool>> sig;
			Assert::IsTrue( sig(5,5).size()==0 );
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				auto erg = sig(5,5);
				Assert::IsTrue( erg.size()==4 );
				Assert::IsTrue( erg[0] );
				Assert::IsTrue( erg[0]==erg[1] );
				Assert::IsTrue( erg[0]==erg[2] );
				Assert::IsTrue( erg[0]==erg[3] );
				erg = sig(5,6);
				Assert::IsTrue( erg.size()==4 );
				Assert::IsFalse( erg[0] );
				Assert::IsTrue( erg[0]==erg[1] );
				Assert::IsTrue( erg[0]==erg[2] );
				Assert::IsTrue( erg[0]==erg[3] );
			}
			sig(5,5);
		}
	};
}

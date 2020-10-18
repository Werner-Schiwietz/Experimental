#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//#include <synchapi.h>
#include <atomic>
#include <mutex>

#include <atlstr.h>

__interface ITest
{
	//virtual ~ITest(){}//lol, compilefehler. darum für mich unbrauchbar
	int f1(int);
	int f2(int,int);
};
namespace test3
{
	//https://stackoverflow.com/questions/49904809/template-function-for-detecting-pointer-like-dereferencable-types-fails-for-ac Answer 4
	//S.F.I.N.A.E.: Substitution Failure Is Not An Error
	template <typename> std::false_type is_pointer_like (unsigned long);
	template <typename T> auto is_pointer_like (int) -> decltype( * std::declval<T>(), std::true_type{} );
	template <typename T> auto is_pointer_like (long) -> decltype( std::declval<T>().operator->(), std::true_type{} );

	template <typename T> using is_pointer_like_arrow_dereferencable = decltype(is_pointer_like<T>(0));
	template <typename T> static auto const is_pointer_like_arrow_dereferencable_v = is_pointer_like_arrow_dereferencable<T>::value;
}
namespace WS
{
	//idee von https://stackoverflow.com/questions/49904809/template-function-for-detecting-pointer-like-dereferencable-types-fails-for-ac Answer 4
	//SFINAE: Substitution Failure Is Not An Error
	template <typename> std::false_type can_dereference(unsigned long);
	template <typename T> auto can_dereference(int) -> decltype( * std::declval<T>(), std::true_type{} );		//kann dereferenziert werden
	//template <typename T> auto can_dereference(long) -> decltype( std::declval<T>()->operator->(), std::true_type{} );	//gibt es den operator -> //hier nicht hilfreich
	template <typename T> using is_dereferenceable = decltype(can_dereference<T>(0));
	template <typename T> static auto const is_dereferenceable_v = is_dereferenceable<T>::value;

	template <typename> std::false_type has_pointer_op(unsigned long);
	template <typename T> auto has_pointer_op(int) -> decltype( std::declval<T>().operator->(), std::true_type{} );//hat operator->
	//template <typename T> auto has_pointer_op(long) -> decltype( * std::declval<T>(), std::true_type{} );		//kann dereferenziert werden? hier nicht hilfreich
	template <typename T> using is_pointer = decltype(has_pointer_op<T>(0));
	template <typename T> static auto const is_pointer_v = is_pointer<T>::value;
}
namespace stdex//brauchbar
{
	//usage stdex::make_shared( datentyp{...} ); also man uebergibt ein refernz oder rv-referenz und spart sich den template-parameter
	template<class _Ty> auto make_shared( _Ty const & v)
	{
		using pointer_element_t = std::remove_reference_t<_Ty>;
		return std::make_shared<pointer_element_t, _Ty const &>( v );
	}
	template<class _Ty> auto make_shared( _Ty && v)
	{
		using pointer_element_t = std::remove_reference_t<_Ty>;
		return std::make_shared<pointer_element_t, _Ty &&>( std::forward<_Ty>(v) );
	}
}
namespace stdex//das bringt nichts, wird nur mit expliciten template-parameter verwendet. einfacher immer dereferenzieren
{
	template<typename _Ty,template <typename _Ty> class _Ptr=std::shared_ptr> auto make_shared( _Ptr<_Ty> const & v_ptr )
	{
		return make_shared( *v_ptr );
	}
	//template<class _Ty> auto make_shared( std::shared_ptr<_Ty> const & v_ptr )
	//{
	//	return make_shared( *v_ptr );
	//}
	//template<class _Ty> auto make_shared( std::shared_ptr<_Ty> && v_ptr )
	//{
	//	return make_shared( *v_ptr );
	//}
}

namespace test1
{
	template<typename T,typename U=void> struct dereferenceable : std::false_type
	{
	};
	template<typename T> struct dereferenceable<T,decltype(*std::declval<T>())> : std::true_type
	{
	};
}


namespace UTAllerei
{
	TEST_CLASS(UT_cpp__interface)
	{
		struct Test : ITest
		{
			bool &alife;
			~Test(){alife=false;}
			Test(bool &alife ):alife(alife=true){}
			int f1(int v) override {return v;}
			int f2(int l,int r) /*override*/ {return l*r;}
		};

	public:
		
		TEST_METHOD(Use_ITest)
		{
			bool TestAlife=false;
			{
				Test test(TestAlife);
				Assert::IsTrue(TestAlife);
				Assert::IsTrue(test.f1(5)==5);
				Assert::IsTrue(test.f2(5,6)==30);
			}
		}
		TEST_METHOD(VirtualDTor_ITest)
		{
			bool TestAlife=false;
			Assert::IsFalse(TestAlife);
			{
				std::unique_ptr<ITest> itest{ new Test{TestAlife} };
				Assert::IsTrue(TestAlife);
				Assert::IsTrue(itest->f1(5)==5);
				Assert::IsTrue(itest->f2(5,6)==30);
			}
			//da ITest keinen virtuellen dtor hat wird der dtor von Test bei std::unique_ptr<ITest> nicht durchlaufen. dumm gelaufen, ist aber c++
			Assert::IsTrue(TestAlife,L"der dtor von 'struct Test' wurde nicht durchlaufen");
		}
	};
	TEST_CLASS(UT_atomic_flag_lock_guard)
	{
		TEST_METHOD(Get_MethodPointer_atomic_flag)
		{
			std::atomic_flag flag;

			//auto x = &std::atomic_flag::clear;
			{
				using clear_v_n  = void (std::atomic_flag::*)(const std::memory_order ) volatile noexcept;//default-parameter???
				clear_v_n clear_fn_ptr = &std::atomic_flag::clear;

				flag.test_and_set();
				(flag.*clear_fn_ptr)( std::memory_order_seq_cst );
			}
			//{
			//	using c_t = decltype(flag.clear);
			//	using clear_v_n  = void (std::atomic_flag::* )(const std::memory_order ) volatile noexcept;//default-parameter???
			//	clear_v_n clear_fn_ptr = &std::atomic_flag::clear;

			//	flag.test_and_set();
			//	(flag.*clear_fn_ptr)( std::memory_order_seq_cst );
			//}
		}
		TEST_METHOD(Get_MethodPointer_own_lock_guard)
		{
			struct atomic_flag_lock_guard
			{
				std::atomic_flag & flag;
				atomic_flag_lock_guard( std::atomic_flag & flag ) : flag(flag)
				{
					while( flag.test_and_set() )
					{
						//Sleep(0);
					}
				}
				~atomic_flag_lock_guard()
				{
					flag.clear();
				}
			};

			std::atomic_flag flag;

			{
				auto locked = atomic_flag_lock_guard(flag);
			}
			{
				auto locked = atomic_flag_lock_guard(flag);
			}
		}
		TEST_METHOD(Get_MethodPointer_std_lock_guard)
		{
			struct atomic_mutex
			{
				std::atomic_flag flag{};

				void lock()
				{
					while(flag.test_and_set()){}
				}
				void unlock()
				{
					flag.clear();
				}

				~atomic_mutex()
				{
					Assert::IsTrue(flag._Storage.is_lock_free());
				}
			};

			atomic_mutex flag;
			Assert::IsTrue(flag.flag._Storage.load()==0);
			{
				auto lock_guard=std::lock_guard(flag);
				Assert::IsFalse(flag.flag._Storage.load()==0);
			}
			Assert::IsTrue(flag.flag._Storage.load()==0);
			{
				auto lock_guard=std::lock_guard(flag);
				Assert::IsFalse(flag.flag._Storage.load()==0);
			}
		}
	};
	TEST_CLASS(UT_CString)
	{
		TEST_METHOD(GetAt)
		{
			CString str;
			try
			{
				auto ch = str.GetAt(0);
				//Assert::Fail(L"nach doku exception erwartet");
				Assert::IsTrue(ch==0);
			}
			catch(...)
			{
			}
			try
			{
				auto ch = str.GetAt(str.GetLength());
				//Assert::Fail(L"nach doku exception erwartet");
				Assert::IsTrue(ch==0);
			}
			catch(...)
			{
			}

			TCHAR const * ptr = _T("hallo");
			str = ptr;
			int i;
			for( i=0; *ptr; ++i, ++ptr )
				Assert::IsTrue(str.GetAt(i)==*ptr);

			try
			{
				auto ch = str.GetAt(i);
				//Assert::Fail(L"nach doku exception erwartet");
				Assert::IsTrue(ch==0);
			}
			catch(...)
			{
			}
			try
			{
				auto ch = str.GetAt(str.GetLength());
				//Assert::Fail(L"nach doku exception erwartet");
				Assert::IsTrue(ch==0);
			}
			catch(...)
			{
			}
		}

	};
	TEST_CLASS(UT_shared_ptr)
	{
		struct A
		{
			using value_t = int;
			~A(){value=0xdddd'dddd;}
			A(){}
			A(A const & r):value(r.value){}
			A(value_t value):value(value){}

			operator value_t&		() &		{return value;} 
			operator value_t const &() const &	{return value;}
 			operator value_t		() &&		{return value;} 

			value_t value{};
		};
		A getA7()
		{
			return A{7};
		}
		TEST_METHOD(UT_struct_A)
		{
			{
				auto a = A{};
				auto const & ca = a;
				{
				#pragma warning(suppress:4239)//warning C4239: nonstandard extension used: 'initializing': conversion from 'UTAllerei::UT_shared_ptr::A' to 'UTAllerei::UT_shared_ptr::A &'
					A & ra = A{6};ra;//keine gute idee, referenz auf ein rvalue, aber der compiler macht den dtor erst später??
					Assert::IsTrue(ra.value==6);

					getA7();//hier wird der destructor von A sofort durchlaufen
				#pragma warning(suppress:4239)//warning C4239: nonstandard extension used: 'initializing': conversion from 'UTAllerei::UT_shared_ptr::A' to 'UTAllerei::UT_shared_ptr::A &'
					A & ra7 = getA7();ra7;//keine gute idee, referenz auf ein rvalue, aber der compiler macht den dtor erst später??
					Assert::IsTrue(ra7.value==7);
				}
				int v = A{};v;
				//int & v = A{};//error C2440: 'initializing': cannot convert from 'UTAllerei::UT_shared_ptr::A' to 'int &'
				int & v2 = a;v2;
				int const & v3 = a;v3;
				//int & v4 = ca;//error C2440: 'initializing': cannot convert from 'const UTAllerei::UT_shared_ptr::A' to 'int &'
				int const & v4 = ca;v4;
			}
		}
		TEST_METHOD(UT_CreateCopy_des_shared_ptr_neue_referenz)
		{
			//using ptr_t = std::shared_ptr<A>;
			//using constptr_t = std::shared_ptr<A const>;

			auto p1 = stdex::make_shared( A{6} );
			auto constp1 = std::make_shared<A const>( A{5} );

			++p1->value;
			decltype(*p1) copyp1_1 = *p1;
			auto copyp1_1_ptr = stdex::make_shared(copyp1_1);
			++copyp1_1_ptr->value;
			auto & constcopyp1_1 = *constp1;
			Assert::IsTrue( constcopyp1_1.value==constp1->value );
			//constcopyp1_1.value++;//error C3490: 'value' cannot be modified because it is being accessed through a const object
			auto constcopyp1_1_ptr = stdex::make_shared(constcopyp1_1);//kopie eines shared_ptr<T const> soll shared_ptr<T> sein
			Assert::IsTrue( constcopyp1_1.value==constcopyp1_1_ptr->value );
			++constcopyp1_1_ptr->value;
			Assert::IsFalse( constcopyp1_1.value==constcopyp1_1_ptr->value );

			auto p3 = stdex::make_shared(*p1);
			++p3->value;

			//++constp1->value;//error C3892: 'constp1': you cannot assign to a variable that is const
			auto p2 = stdex::make_shared(*constp1);
			++p2->value;
		}
		TEST_METHOD(UT_CreateCopy_des_shared_ptr_neue_referenz2)
		{
			auto ptr = std::make_shared<A>(5);
			//auto ptr2 = std::make_shared<A>(ptr);//compile-fehler
			auto ptr3 = stdex::make_shared(ptr);//macht leider einen shared_ptr<shared_ptr<A>>, und keinen shared_ptr<A>
			auto ptr4 = stdex::make_shared<decltype(ptr)::element_type>(ptr);//zu kompliziert, fehleranfällig. macht keinen shared_ptr<shared_ptr<A>>, sondern einen shared_ptr<A>

			auto ptr2 = stdex::make_shared(*ptr);//so am einfachsten, macht keinen shared_ptr<shared_ptr<A>>, sondern einen shared_ptr<A>
			Assert::IsTrue(ptr2!=ptr);
			Assert::IsTrue(*ptr2==*ptr);
			(*ptr)++;
			Assert::IsTrue(*ptr2!=*ptr);
		}
		TEST_METHOD(UT_kopie_des_shared_ptr_gleiche_referenz)
		{
			auto ptr = std::make_shared<int>(5);
			auto ptr2 = ptr;
			std::shared_ptr<decltype(ptr)::element_type const> constptr2 = ptr;//über constptr2 sind keine änderung möglich, aber änderungen an z.b. ptr2 ändern auch constptr2
			auto  test=[&]
			{
				Assert::IsTrue( ptr==ptr2);//pointer sind gleich
				Assert::IsTrue( ptr==constptr2);//pointer sind gleich
				Assert::IsTrue( *ptr==*ptr2);//inhalt dann natürlich auch
				Assert::IsTrue( *ptr==*constptr2);//inhalt dann natürlich auch
			};
			test();
			(*ptr)++;
			test();
			(*ptr2)++;
			test();
			//(*constptr2)++;//error C3892: 'constptr2': you cannot assign to a variable that is const
		}
		TEST_METHOD(UT_shared_ptr_is_pointer_test)
		{
			static_assert(std::is_pointer<int>::value==false);
			static_assert(std::is_pointer_v<int*> );
			//static_assert(std::is_pointer_v<std::unique_ptr<int>>==false);//lustiger error error C2947: expecting '>' to terminate template-argument-list, found '>>='
			static_assert(std::is_pointer_v<std::unique_ptr<int>> == false);//so geht es
			static_assert(std::is_pointer<std::unique_ptr<int>>::value==false);//nicht das gewünscht ergebnis


			std::remove_reference_t<decltype(*std::declval<int*>())> i = 5;i;
			static_assert(test1::dereferenceable<int>::value == false );
			static_assert(test1::dereferenceable<int*,decltype(*std::declval<int*>())>::value == true );


			//static_assert(test1::dereferenceable<int*>::value == true );

			//static_assert(test::is_pointer_like_arrow_dereferencable<int>::value == false );
			//static_assert(std::is_same<std::true_type,decltype(test::dereferenceable<int*>())>::value == true );
			//static_assert(test::dereferenceable<int*,decltype(*std::declval<int*>())>::value == true );


			static_assert(WS::is_dereferenceable<int>::value == false );
			static_assert(WS::is_dereferenceable<int const>::value == false );
			static_assert(WS::is_dereferenceable<int volatile>::value == false );
			static_assert(WS::is_dereferenceable<int const volatile>::value == false );

			static_assert(WS::is_dereferenceable<int*>::value == true );
			static_assert(WS::is_dereferenceable<int const *>::value == true );
			static_assert(WS::is_dereferenceable<int volatile *>::value == true );
			static_assert(WS::is_dereferenceable<int const volatile *>::value == true );

			static_assert(WS::is_dereferenceable<std::unique_ptr<int>>::value == true );
			static_assert(WS::is_dereferenceable<std::unique_ptr<int const>>::value == true );

			static_assert(WS::is_dereferenceable<std::shared_ptr<int>>::value == true );
			static_assert(WS::is_dereferenceable<std::shared_ptr<int const>>::value == true );

		}
		TEST_METHOD(UT_stdex_make_shared)
		{
			{
				auto str = CString{L"hallo"};
				static_assert( WS::is_dereferenceable_v<CString> == true );
				auto str_data = *str;
				Assert::IsTrue( str_data==L'h');
				static_assert( WS::is_pointer_v<CString> == false);
				//auto str_dataptr = str.operator->();//error C2039: '->': is not a member of 'ATL::CStringT<wchar_t,ATL::StrTraitATL<wchar_t,ATL::ChTraitsCRT<wchar_t>>>'

				auto x = stdex::make_shared(str);//lv
				auto x1 = stdex::make_shared(CString{L"hallo"});//rv
				static_assert( std::is_same<decltype(x),decltype(x1)>::value);
				Assert::IsTrue( *x==L"hallo" );
				auto xx = stdex::make_shared(x);//shard_ptr<shared_ptr<CString>>
				Assert::IsTrue( **xx==L"hallo" );
				auto xxx = stdex::make_shared(*x);
				Assert::IsTrue( *x==L"hallo" );
			}
			{
				auto x = stdex::make_shared(CString{L"hallo"});
				Assert::IsTrue( *x==L"hallo" );
			}

		}
		TEST_METHOD(UT_test3)
		{			
			struct op_value
			{
				int i={};
				char const *p = {};
			};
			struct op_pointer : op_value
			{
				op_value * operator->(){ return this;}
			};
			struct op_deref : op_value
			{
				op_value & operator*(){ return *this;}
			};
			struct op_deref_pointer : op_deref, op_pointer
			{
			};

			static_assert(WS::is_dereferenceable<op_deref>::value == true );
			static_assert(WS::is_dereferenceable<op_deref_pointer>::value == true );
			static_assert(test3::is_pointer_like_arrow_dereferencable<op_deref>::value == true );
			static_assert(test3::is_pointer_like_arrow_dereferencable<op_deref_pointer>::value == true );


			static_assert(WS::is_dereferenceable<op_pointer>::value == false );

			op_pointer v1;
			(*v1.operator->()) = {4};
			decltype( std::declval<op_pointer>().operator->() ) pi = v1.operator->();
			++(*pi).i;
			Assert::IsTrue( v1.i == 5 );
			
			static_assert(WS::is_pointer<op_pointer>::value == true );
			static_assert(WS::is_pointer<op_deref>::value == false );
			static_assert(WS::is_pointer<op_deref_pointer>::value == true );

			//static_assert(test3::is_pointer_like_arrow_dereferencable<op_pointer>::value == false );//error C2668: 'test3::is_pointer_like': ambiguous call to overloaded function
			static_assert(test3::is_pointer_like_arrow_dereferencable<op_deref>::value == true );//??falsch 
			static_assert(test3::is_pointer_like_arrow_dereferencable<op_deref_pointer>::value == true );

			
		}
	};
}

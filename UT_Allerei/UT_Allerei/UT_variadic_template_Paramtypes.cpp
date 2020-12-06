#include "pch.h"

#include "..\..\..\WernersTools\headeronly\type_list.h"

#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;



#include <vector>
#include <string>
#include <tuple>


namespace UTAllerei
{
	TEST_CLASS(UT_variadic_template_paramtype)
	{
	public:
		TEST_METHOD(UT_4Param)
		{
			//static_assert( std::is_same_v<WS::get_type<0,int,short,char const *,std::string &>::type,short> );//error C2607: static assertion failed
			//static_assert( std::is_same_v<WS::get_type<5,int,short,char const *,std::string &>::type,int> );//5 ist hier ungültig error C2976: 'WStest::get_type': too few template arguments
			static_assert( std::is_same_v<WS::get_type<0,int,short,char const *,std::string &>::type,int> );
			static_assert( std::is_same_v<WS::get_type<1,int,short,char const *,std::string &>::type,short> );
			static_assert( std::is_same_v<WS::get_type<2,int,short,char const *,std::string &>::type,const char*> );
			static_assert( std::is_same_v<WS::get_type<3,int,short,char const *,std::string &>::type,std::string&> );

			std::string s{"hallo"};
			WS::get_type_t<0,int,short,char const *,std::string &> v0 {5};v0;
			WS::get_type_t<1,int,short,char const *,std::string &> v1 {6};v1;
			WS::get_type_t<2,int,short,char const *,std::string &> v2 {s.c_str()};v2;
			WS::get_type_t<3,int,short,char const *,std::string &> v3 = s;//xxx ist vom type std::string&
			v3 = "Welt";
			Assert::IsTrue( v3==s );
		}
		TEST_METHOD(UT_type_list)
		{
			std::string s{"hallo"};

			using typelist = WS::typelist<int,short,char const *,std::string &>;
			static_assert( typelist::count==4 );
			typelist::type<0> v1 {};v1;
			typelist::type<1> v2 {};v2;
			typelist::type<2> v3 {};v3;
			//typelist::type<3> v4 {};v4;//error C2440: 'initializing': cannot convert from 'initializer list' to 'std::string &'
			typelist::type<3> v4 {s};v4;
			//typelist::type<4> v5 {};v5;	//error C2976: 'WStest::get_type': too few template arguments 
											//error C2938: 'WStest::type_list<int,short,const char *,std::string &>::type' : Failed to specialize alias template

		}
		TEST_METHOD(UT_type_list_tuple)
		{
			std::string s{"hallo"};

			{
				using typelist = WS::typelist<int,short,char const *, std::string &>;
				static_assert( typelist::count==4 );

				typelist::tuple_t tuple{1,2i16,"hallo",s};
				Assert::IsTrue(std::get<0>(tuple)==1);
				Assert::IsTrue(std::get<1>(tuple)==2);
				#pragma warning(suppress:4130)
				Assert::IsTrue(std::get<2>(tuple)=="hallo");//pointer vergleich je nach projekteinstellung ok
				Assert::IsTrue(std::get<3>(tuple)==s);
			}

		}
	};
}

#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <vector>
#include <set>


namespace 
{
	template<template<typename...> typename T, typename ... TTs> 
	auto use_set( T<TTs...> const & v )//->std::enable_if<std::is_same<T<TTs...>,std::set<TTs...>>::value,void>
	{
		v;
		static_assert( std::is_same<T<TTs...>,std::set<TTs...>>::value, "funktion nur fuer std::set" );
	}

	//template<typename,typename...> struct is_ : std::false_type{};
	//template<template<typename...> typename T, typename ... TTs> struct is_ : std::true_type{};

	template<typename ... TTs> 
	auto use_( std::shared_ptr<TTs...> & Ptr )
	{
		return Ptr.get();
	}
	template<typename ... TTs> 
	auto use_( std::set<TTs...> & set )
	{
		return set.begin();
	}
}


namespace UTAllerei
{
	TEST_CLASS(UT_parameter_tester)
	{
		TEST_METHOD(UT_per_template_template_static_assert)
		{
			using container_value_t = int volatile;
			struct less{
				bool operator()( container_value_t const &, container_value_t const &) const;
			};
			{
				auto set = std::set<int>{};
				use_set( set );
			}
			{
				auto set = std::set<container_value_t,less>{};
				use_set( set );
			}
			{
				std::shared_ptr<std::string> value;
				//use_set( value );//error C2338: funktion nur fuer std::set
			}
		}
		TEST_METHOD(UT_per_template_parameter)
		{
			using type = std::string;
			{
				std::shared_ptr<type> value;
				Assert::IsFalse( use_( value ) );
			}
			{
				struct deleter{
					void operator()( type * p ) const { delete p;}
				};
				std::shared_ptr<std::string> value{ new std::string{"hallo"}, deleter{} };
				Assert::IsTrue( use_( value ) );
			}
			{
				auto vec = std::vector<int>{};
				//use_( vec );				//error C2672: 'use_': no matching overloaded function found
											//error C2784: 'auto use_(std::set<TTs...> &)': could not deduce template argument for 'std::set<TTs...> &' from 'std::vector<int,std::allocator<int>>'

			}
			{
				auto set = std::set<int>{};
				Assert::IsTrue( use_( set ) == set.end());
				set.insert(5);
				set.insert(3);
				Assert::IsTrue( use_( set ) != set.end() );
				Assert::IsTrue( *use_( set ) == 3 );
			}
			{
				using value_t = int;
				struct less{ bool operator() (value_t const & l,value_t const & r ) const {return r<l;} };
				auto set = std::set<int,less>{};
				Assert::IsTrue( use_( set ) == set.end());
				set.insert(5);
				set.insert(3);
				Assert::IsTrue( use_( set ) != set.end() );
				Assert::IsTrue( *use_( set ) == 5 );
			}
		}
	};
}

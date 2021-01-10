#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "ReadWrite.h"

#include "..\..\..\WernersTools\headeronly\type_list.h"

#include "ReadWrite_char_t.h"
#include "ReadWrite_char_t_array.h"

#include <afx.h>//CFile

template<typename enum_type, typename ... types>
struct struct_name
{	
	using typelist = WS::typelist<types...>;
	using tuple_t = typename typelist::tuple_t;
	using member = enum_type;
	tuple_t values{};

	template<member index> auto & operator()() &		{return std::get<index>(this->values);}//funktioniert, aber bringt für den aufrufer nichts, nur unverständnis. aufruf per v.[template ]operator()<0>(); [template ] optinal evtl. bei einigen compilern nötig
	template<member index> auto   operator()()  &&		{return std::get<index>(this->values);}//funktioniert, aber bringt für den aufrufer nichts, nur unverständnis
	template<member index> auto & access() &			{return std::get<index>(this->values);}
	template<member index> auto   access() &&			{return std::get<index>(this->values);}

	template<member index,typename T> struct_name& set(T&&value) &	
	{
		std::get<index>(this->values) = std::forward<T>(value);
		return *this;
	}
	template<member index,typename T> struct_name set(T&&value) &&
	{
		std::get<index>(this->values) = std::forward<T>(value);
		return std::move(*this);
	}
};
template<typename io_interface, template<typename...> typename struct_name,typename enum_type,typename ... types> void WriteData( io_interface && io, struct_name<enum_type,types...> const & value  )
{
	io;value;
	

	//std::make_index_sequence<sizeof...(types)> indexs;
	//auto write =[&](auto const & v ){ WriteData(std::forward<io_interface>(io),v); };
	//(...,write(value.values.access<indexs>));  
	//auto write = [&]( auto const & value){WriteData(std::forward<io_interface>(io),value);}
	//std::apply( write, value.values );
}
template<typename io_interface, template<typename...> typename struct_name,typename enum_type,typename ... types> void ReadData( io_interface && io, struct_name<enum_type,types...> & value  )
{
	io;value;
	std::make_index_sequence<sizeof...(types)> x;x;
	//auto read= [&]( auto & value){ReadData(std::forward<io_interface>(io),value);}
	//std::apply( read, value.values );
}

namespace enumNS
{
	enum :unsigned __int16{f,l};
}

#define MAKE_STRUCT(struct_name, type, type_name, ...)										\
struct struct_name																			\
{																							\
	enum member{type_name};																	\
	std::tuple<type> values{};																\
	template<member index> auto & access()		 {return std::get<index>(this->values);}	\
	template<member index> auto & access() const {return std::get<index>(this->values);}	\
}

struct Member
{
	static size_t constexpr n0 = 0;
	static size_t constexpr n1 = 1;
	std::tuple<int,std::string> member;

	Member(){};
	Member(int const & v0,std::string const & v1):member(v0,v1){}
	template<size_t name> auto& get() & {return std::get<name>(member);}
	template<size_t name> auto const & get() const & {return std::get<name>(member);}
};

namespace UT_TupleMember
{
	TEST_CLASS(UT_TupleMember)
	{
	public:
		MAKE_STRUCT(test_t,int,i,char const *,text) struct1;
		TEST_METHOD(enum_namespace)
		{
			#pragma warning(suppress:26812)
			auto x = enumNS::f;
			decltype(enumNS::f) x2 = x;
			decltype(x2) x3 = enumNS::l;x3;
		}
		TEST_METHOD(UT_struct_name_WriteData_ReadData)
		{
			enum member{type_name0,type_name1};
			using s_type = struct_name<member,int,char const *>;
			auto s = s_type{}.set<s_type::member::type_name0>(5).set<s_type::member::type_name1>("hallo");

			CMemFile file;
			WriteData( file, s );

			file.Seek(0,CFile::begin);

			decltype(s) s_read;

			ReadData( file, s_read );

			Assert::IsTrue( s_read.access<decltype(s)::member::type_name0>() == s.access<decltype(s)::member::type_name0>());
			Assert::IsFalse( s_read.access<decltype(s)::member::type_name1>() == s.access<decltype(s)::member::type_name1>());
			Assert::IsTrue( stringcmp(s_read.access<decltype(s)::member::type_name1>(),s.access<decltype(s)::member::type_name1>())==0 );

		}
		TEST_METHOD(TestMethod3)
		{
			enum member{type_name0,type_name1};
			using s_type = struct_name<member,int,char const *>;
			auto s = s_type{}.set<s_type::member::type_name0>(5).set<s_type::member::type_name1>("hallo");
			Assert::IsTrue( s.operator()<s_type::member::type_name0>()==5);
			Assert::IsTrue( s.template operator()<s_type::member::type_name0>()==5);
			Assert::IsTrue( s.template access<s_type::member::type_name0>()==5);
			Assert::IsTrue( s.access<s_type::member::type_name0>()==5);
			Assert::IsTrue( s.access<0>()==5);
			//
			Assert::IsTrue( strcmp(s.operator()<s_type::member::type_name1>(),"hallo")==0);
			#pragma warning(suppress:4130)//warning C4130: '==': logical operation on address of string constant
			Assert::IsTrue( s.operator()<s_type::member::type_name1>()=="hallo");
			#pragma warning(suppress:4130)//warning C4130: '==': logical operation on address of string constant
			Assert::IsTrue( s.template operator()<s_type::member::type_name1>()=="hallo");
			//Assert::IsTrue( s.template <struct_name::type_name1>()=="hallo");
		}
		TEST_METHOD(TestMethod2)
		{
			std::get<test_t::member::i>(struct1.values) = 5;
			Assert::IsTrue( std::get<test_t::member::i>(struct1.values)==5);
			Assert::IsTrue( std::get<0>(struct1.values)==5);
			Assert::IsTrue( struct1.access<decltype(struct1)::i>()==5);
			Assert::IsTrue( struct1.access<test_t::i>()==5);
			Assert::IsTrue( struct1.access<0>()==5);
			struct1.access<test_t::member::i>() = 6;
			Assert::IsTrue( struct1.access<test_t::i>()==6);
			//Assert::IsTrue( struct1.access<100>()==5);//ungültiger index. error C2672: 'UTTupleMember::UTTupleMember::test_t::access': no matching overloaded function found ... message : 'index=100'
			//Assert::IsTrue( struct1.access<test_t::member::text>()=="hallo");//noch nicht fertig
		}
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
		TEST_METHOD(UT_baseclass_ref_virtuell_method)
		{
			struct A
			{
				virtual char foo(){return 'A';}
			};
			struct B : A
			{
				virtual char foo(){return 'B';}
			};

			A a;
			B b;
			A& refA = a;
			A& refB = b;
			Assert::IsTrue( a.foo()== refA.foo() );
			Assert::IsTrue( b.foo()== refB.foo() );
		}
	};
}

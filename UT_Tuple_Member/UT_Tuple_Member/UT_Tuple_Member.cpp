#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "ReadWrite.h"

#include "..\..\..\WernersTools\headeronly\type_list.h"

#include "ReadWrite_char_t.h"
#include "ReadWrite_char_t_array.h"

#include "tuple_struct.h"

#include "derefernce.h"

#include <afx.h>//CFile

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
			[[gsl::suppress(26812)]]
			{
				auto x = enumNS::f;x;
			}
			{
				#pragma warning(suppress:26812)
				auto x = enumNS::f;
				decltype(enumNS::f) x2 = x;
				decltype(x2) x3 = enumNS::l;x3;
			}
		}
		TEST_METHOD(UT_struct_name_WriteData_ReadData)
		{
			enum names{type_name0,type_name1};
			using s_type = tuple_struct<names,int,std::unique_ptr<char[]>>;
			auto str2=std::unique_ptr<char[]>{_strdup("hallo")};
			std::unique_ptr<char[]> str;
			auto & rstr = str;
			rstr = std::move(str2);

			auto s = s_type{}.set<s_type::member::type_name0>(5);//.set<s_type::member::type_name1>(std::move(str));

			CMemFile file;
			WriteData( file, s );
			s.set<s_type::member::type_name1>( std::unique_ptr<char[]>(_strdup("hallo")) );
			WriteData( file, s );


			file.Seek(0,CFile::begin);

			decltype(s) s_read;

			ReadData( file, s_read );

			Assert::IsTrue( s_read.access<decltype(s)::member::type_name0>() == s.access<decltype(s)::member::type_name0>());
			Assert::IsTrue( s_read.access<decltype(s)::member::type_name1>() == s_type{}.access<decltype(s)::member::type_name1>() );//nullptr sind gleich
			Assert::IsTrue(  dereferenced::equal( s_read.access<decltype(s)::member::type_name1>(),s_type{}.access<decltype(s)::member::type_name1>()) );

			ReadData( file, s_read );
			Assert::IsTrue( s_read.access<decltype(s)::member::type_name0>() == s.access<decltype(s)::member::type_name0>());
			Assert::IsFalse( s_read.access<decltype(s)::member::type_name1>() == s.access<decltype(s)::member::type_name1>());//pointer sind unterschiedlich
			Assert::IsTrue(  dereferenced::equal( s_read.access<decltype(s)::member::type_name1>(),s.access<decltype(s)::member::type_name1>()) );//inhalt ist gleich



		}
		TEST_METHOD(TestMethod_enum_zu_wenig_eintraege)
		{
			enum class member{type_name0,type_name1};//??weniger enum-values als type, unsinn geht aber leider ohgne warning
			using s_type = tuple_struct<member,int,char const *,short>;
			auto s = s_type{}.set<s_type::member::type_name0>(5).set<s_type::member::type_name1>("hallo").set<member{2}>(6i16);//daemlich, geht aber
			Assert::IsTrue( s.template access<s_type::member::type_name0>()==5);
			Assert::IsTrue( s.access<s_type::member::type_name0>()==5);//getter
			Assert::IsTrue( (s.access<s_type::member::type_name0>()=7) == 7 );//setter
			Assert::IsTrue( s.access<s_type::member::type_name0>()==7);//getter
			Assert::IsTrue( s.access<s_type::member::type_name1>()==std::string("hallo"));
			Assert::IsTrue( s.access<member{2}>()==6);
		}
		TEST_METHOD(TestMethod3)
		{
			enum member{type_name0,type_name1};
			using s_type = tuple_struct<member,int,char const *>;
			auto s = s_type{}.set<s_type::member::type_name0>(5).set<s_type::member::type_name1>("hallo");
			//Assert::IsTrue( s.operator()<s_type::member::type_name0>()==5);
			//Assert::IsTrue( s.template operator()<s_type::member::type_name0>()==5);
			Assert::IsTrue( s.template access<s_type::member::type_name0>()==5);
			Assert::IsTrue( s.access<s_type::member::type_name0>()==5);
			Assert::IsTrue( s.access<0>()==5);
			//
			//Assert::IsTrue( strcmp(s.operator()<s_type::member::type_name1>(),"hallo")==0);
		#pragma warning(suppress:4130)//warning C4130: '==': logical operation on address of string constant
			//Assert::IsTrue( s.operator()<s_type::member::type_name1>()=="hallo");
		#pragma warning(suppress:4130)//warning C4130: '==': logical operation on address of string constant
			//Assert::IsTrue( s.template operator()<s_type::member::type_name1>()=="hallo");
			//Assert::IsTrue( s.template <tuple_struct::type_name1>()=="hallo");
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

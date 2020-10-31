#include "pch.h"
#include "CppUnitTest.h"

#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace WS
{
	struct compare_bool
	{
		virtual bool Valid() const = 0;
		operator bool() const { return Valid();}
		bool operator!() const { return !operator bool();}
		bool operator==(bool r) const { return operator bool()==r;}
		bool operator!=(bool r) const { return operator bool()!=r;}
		friend bool operator==(bool l, compare_bool const & r) { return r==l;}
		friend bool operator!=(bool l, compare_bool const & r) { return r!=l;}
	};
	template<typename value_type> struct return_type : compare_bool
	{
		using value_t = value_type;
		//using compare_bool::operator bool;
		value_t		value {};
		bool		valid = false;

		return_type() noexcept(noexcept(value_t{})) {}
		return_type(value_t const & value) noexcept(noexcept(value_t(value))) : value(value),valid(true) {}
		return_type(value_t && value) noexcept(noexcept(value_t(std::move(value)))): value(std::move(value)),valid(true) {}

		//bool operator==(value_t const & r) const { return toValueType()==r;}
		//bool operator!=(value_t const & r) const { return toValueType()!=r;}

		auto & toValueType() const &					{return this->value;} 
		auto & toValueType() &							{return this->value;} 
		auto && toValueType() &&						{return std::move(this->value);}//ohoh nutzung ohne prüfung ob valid? value nach aufruf evtl. leer aber valid-status bleibt ggf. true
		explicit operator value_t const & () const &	{return this->value;} 
		explicit operator value_t & () &				{return this->value;} 
		explicit operator value_t && () &&				{return std::move(this->value);}//ohoh nutzung ohne prüfung ob valid? value nach aufruf evtl. leer aber valid-status bleibt ggf. true
		bool Valid() const override						{return this->valid;}
	};
}

namespace UT_compare_bool
{
	TEST_CLASS(UT_ret_type)
	{
	public:
		TEST_METHOD(UT_test_compare_bool)
		{
			struct A : WS::compare_bool
			{
				A(){}
				A(bool valid):valid(valid){}
				int  value {};
				bool valid = false;
				bool Valid() const override { return this->valid;}
			};

			Assert::IsFalse(A{});
			Assert::IsFalse(A{}==true);
			Assert::IsFalse(true==A{});
			Assert::IsTrue(!A{});
			Assert::IsTrue(A{}!=true);
			Assert::IsTrue(true!=A{});

			Assert::IsTrue(A{true});
			Assert::IsTrue(A{true}==true);
			Assert::IsTrue(true==A{true});
			Assert::IsFalse(!A{true});
			Assert::IsFalse(A{true}!=true);
			Assert::IsFalse(true!=A{true});
		}
		TEST_METHOD(UT_test_ret_type_int)
		{
			struct A : WS::return_type<int>
			{
				using base_t = WS::return_type<int>;
				using base_t::base_t;
			};

			Assert::IsFalse(A{});
			Assert::IsFalse(A{}==true);
			Assert::IsFalse(true==A{});
			Assert::IsTrue(!A{});
			Assert::IsTrue(A{}!=true);
			Assert::IsTrue(true!=A{});

			Assert::IsTrue(A{1});
			Assert::IsTrue(A{2}==true);
			Assert::IsTrue(true==A{3});
			Assert::IsFalse(!A{4});
			Assert::IsFalse(A{5}!=true);
			Assert::IsFalse(true!=A{6});
			if( auto a = A{6} )
			{
				Assert::IsTrue( (A::value_t)a==6 );
			}
			Assert::IsFalse(true!=A{6});
		}
		TEST_METHOD(UT_test_ret_type_string)
		{
			struct A : WS::return_type<std::string>
			{
				using base_t = WS::return_type<std::string>;
				using base_t::base_t;
			};

			Assert::IsFalse(A{});
			Assert::IsFalse(A{}==true);
			Assert::IsFalse(true==A{});
			Assert::IsTrue(!A{});
			Assert::IsTrue(A{}!=true);
			Assert::IsTrue(true!=A{});

			Assert::IsTrue(A{"hallo"});
			Assert::IsTrue(A{"welt"}==true);
			Assert::IsTrue(true==A{"Hallo"});
			Assert::IsFalse(!A{"Welt"});
			Assert::IsFalse(A{""}!=true);
			Assert::IsFalse(true!=A{std::string{' '}});
			if( auto a = A{"s"} )
			{
				Assert::IsTrue( a=="s" );
			}
			Assert::IsFalse(true!=A{"6"});
		}
		TEST_METHOD(UT_test_ret_type_string_return)
		{
			auto fn =[](char const * text )->WS::return_type<std::string>{if(text)return {text};return {}; };

			if( auto value = fn(nullptr) )
			{
				Assert::Fail();
			}
			else
			{
				Assert::IsTrue(decltype(value)::value_t{} == value.toValueType() );
				Assert::AreEqual(decltype(value)::value_t{}, value.toValueType() );

			}
			if( auto value = fn("Hallo") )
			{
				Assert::IsTrue( value == "Hallo");//vorsicht boolvergleich
				Assert::IsTrue( value == "Welt");//vorsicht boolvergleich
				//Assert::IsTrue( value == std::string{"Hallo"});//error C2678: binary '==': no operator found which takes a left-hand operand of type 'std::string' (or there is no acceptable conversion)
				//Assert::IsTrue( value == std::string{"Welt"});//error C2678: binary '==': no operator found which takes a left-hand operand of type 'std::string' (or there is no acceptable conversion)
				Assert::IsTrue( value.toValueType() == std::string{"Hallo"});
				Assert::IsTrue( value.toValueType() != std::string{"Welt"});
				Assert::IsTrue( value.toValueType() == "Hallo");
				Assert::IsFalse( value.toValueType() == "Welt");

				auto s = value.toValueType();
				auto const & constvalue = value;
				auto cs = constvalue.toValueType();
				Assert::IsTrue( s == cs );

				Assert::IsTrue( value.toValueType() == s);

				std::move(value).toValueType();//passiert noch nichts
				Assert::IsTrue( value );
				Assert::IsTrue( value.toValueType() == "Hallo");
				s = std::move(value).toValueType();				//value.value ist danach leer aber immer noch valid, weil 
				Assert::IsTrue( value );
				Assert::IsTrue( value.toValueType().empty() );

				Assert::IsTrue( value.toValueType() != s);
				Assert::IsTrue( s == "Hallo" );

			}
			else
			{
				Assert::Fail();
			}
		}
		TEST_METHOD(UT_test_ret_type_bool)
		{
			struct A : WS::return_type<bool>//mit bool funktioniert das nicht wirklich gut
			{
				using base_t = WS::return_type<bool>;
				using base_t::base_t;
			};

			//Assert::IsFalse(A{});//error C2664: 'void Microsoft::VisualStudio::CppUnitTestFramework::Assert::IsFalse(bool,const wchar_t *,const Microsoft::VisualStudio::CppUnitTestFramework::__LineInfo *)': cannot convert argument 1 from 'UT_compare_bool::UT_ret_type::UT_test_ret_type3::A' to 'bool'
			Assert::IsFalse(A{}==true);
			Assert::IsFalse(true==A{});
			Assert::IsTrue(!A{});
			Assert::IsTrue(A{}!=true);
			Assert::IsTrue(true!=A{});

			//Assert::IsTrue(A{0});//error C2664: 'void Microsoft::VisualStudio::CppUnitTestFramework::Assert::IsTrue(bool,const wchar_t *,const Microsoft::VisualStudio::CppUnitTestFramework::__LineInfo *)': cannot convert argument 1 from 'UT_compare_bool::UT_ret_type::UT_test_ret_type3::A' to 'bool'
			Assert::IsTrue(A{0}==true);
			Assert::IsTrue(true==A{0});
			Assert::IsFalse(!A{0});
			Assert::IsFalse(A{0}!=true);
			Assert::IsFalse(true!=A{0});
			auto a1 = A{false};
			if( a1.Valid() )
			{
				Assert::IsTrue( a1.toValueType()==false );
			}
			//if( auto a = A{false} )//error C2451: conditional expression of type 'UT_compare_bool::UT_ret_type::UT_test_ret_type3::A' is illegal
			//{
			//	Assert::IsTrue( a1.toValueType()==false );
			//}
		}
		TEST_METHOD(UT_test_ret_type_noexcept)
		{
			struct X
			{
				X() noexcept {}
				X( X const & ) noexcept(false){} 
				X( X && ) noexcept{} 
			};
			struct Z
			{
				Z() {}
				Z( Z const & ) {} 
				Z( Z && ) noexcept{} 
			};

			{
				auto noexc1 = noexcept( X() );
				auto noexc2 = noexcept( X(std::declval<X>()) );
				auto noexc3 = noexcept( X(std::declval<X const>() ) );
				Assert::IsTrue( noexc1 );
				Assert::IsTrue( noexc2 );
				Assert::IsFalse( noexc3 );

				auto noexc1_ = noexcept( WS::return_type<X>() );
				auto noexc2_ = noexcept( WS::return_type<X>(std::move(std::declval<X>())) );
				auto noexc3_ = noexcept( WS::return_type<X>( std::declval<X const>() ) );
				Assert::AreEqual(noexc1,noexc1_);
				Assert::AreEqual(noexc2,noexc2_);
				Assert::AreEqual(noexc3,noexc3_);
			}
			{
				auto noexc1 = noexcept( Z() );								//Z()
				auto noexc2 = noexcept( Z(std::move(std::declval<Z>())) );	//Z(Z&&)
				auto noexc3 = noexcept( Z( std::declval<Z const>() ) );		//Z(Z const &)
				Assert::IsFalse( noexc1 );
				Assert::IsTrue( noexc2 );
				Assert::IsFalse( noexc3 );

				auto noexc1_ = noexcept( WS::return_type<Z>() );							//Z()
				auto noexc2_ = noexcept( WS::return_type<Z>(std::declval<Z>()) );			//Z(Z&&)
				auto noexc3_ = noexcept( WS::return_type<Z>(std::declval<Z const>() ) );	//Z(Z const &)
				Assert::AreEqual(noexc1,noexc1_);
				Assert::AreEqual(noexc2,noexc2_);
				Assert::AreEqual(noexc3,noexc3_);
			}
		}
	};
}

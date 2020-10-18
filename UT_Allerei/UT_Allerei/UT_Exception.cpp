#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void out( char const * text)
{
	Logger::WriteMessage( text );
}


namespace UTAllerei
{
	struct A : std::exception
	{
		using std::exception::exception;
		virtual char const * classname() const { return __FUNCTION__; }
	};
	struct B : A
	{
		using A::A;
		virtual char const * classname() const { return __FUNCTION__; }
	};
	enum enumtest_how { rethrow,rethrow_ref };
	void catch_std_exception( enumtest_how test_how )
	{
		try
		{
			try
			{
				throw B("throw B");
			}
			catch( std::exception & e)
			{
				switch(test_how)
				{
				case rethrow_ref:
					throw e;
				}
				throw;
			}
		}
		catch( B & e)
		{
			out( e.classname() );
			out( e.what() );
		}
		catch( A & e)
		{
			out( e.classname() );
			out( e.what() );
		}
		catch( std::exception & e)
		{
			out( e.what() );
		}
	}
	void catch_A( enumtest_how test_how )
	{
		try
		{
			try
			{
				throw B("throw B");
			}
			catch( A & e)
			{
				switch(test_how)
				{
				case rethrow_ref:
					throw e;
				}
				throw;
			}
		}
		catch( B & e)
		{
			out( e.classname() );
			out( e.what() );
		}
		catch( A & e)
		{
			out( e.classname() );
			out( e.what() );
		}
		catch( std::exception & e)
		{
			out( e.what() );
		}
	}

	TEST_CLASS(UT_throw_exception)
	{

	public:

		TEST_METHOD(throw_recatch)
		{
			out("catch std::exception per rethrow");
			catch_std_exception(enumtest_how::rethrow);
			out("\ncatch std::exception per rethrow reference on exception");
			catch_std_exception(enumtest_how::rethrow_ref);
			out("");
			out("\n");
			out("catch A per rethrow");
			catch_A(enumtest_how::rethrow);
			out("\ncatch A per rethrow reference on exception");
			catch_A(enumtest_how::rethrow_ref);
		}
	};
}

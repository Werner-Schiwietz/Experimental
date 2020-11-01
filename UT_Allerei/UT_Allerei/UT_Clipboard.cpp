#include "pch.h"
#include "CppUnitTest.h"

#include <windows.h>
#include <winuser.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace WS
{
	struct Clipboard
	{
		bool openflag = false;
		~Clipboard(){ close();}
		Clipboard(){}
		Clipboard(HWND handle){open(handle);}

		Clipboard(Clipboard const &) = delete;
		Clipboard(Clipboard && r){ swap(r);};
		Clipboard& operator=(Clipboard const &) = delete;
		Clipboard& operator=(Clipboard && r ) & { Clipboard{std::move(r)}.swap(*this);}
		void swap(Clipboard & r)
		{
			std::swap( this->openflag, r.openflag );
		}

		bool is_open() const { return this->openflag;}
		operator bool() const {return is_open();}

		bool open(HWND hwnd)
		{
			//if( is_open() ) //sinnlos
			//	return true;
			return this->openflag = OpenClipboard(hwnd);
		}
		bool open(){return open(nullptr);}

		bool close() 
		{
			if( this->is_open() )
			{
				this->openflag=false;
				return !! CloseClipboard();
			}
			return false;
		}
	};
}

namespace UT_Clipboard
{
	TEST_CLASS(UT_Clipboard)
	{
	public:
		TEST_METHOD(UT_test_openclipboard)
		{
			Assert::IsTrue( OpenClipboard(nullptr) );
			Assert::IsTrue( EmptyClipboard() );
			Assert::IsTrue( OpenClipboard(nullptr) );
			Assert::IsTrue( EmptyClipboard() );
			Assert::IsTrue( CloseClipboard() );
			Assert::IsFalse( EmptyClipboard() );//unerwartet, da in der hilfe steht für jeden openclipboard braucht es ein  CloseClipboard
			Assert::IsTrue( OpenClipboard(nullptr) );
			Assert::IsTrue( EmptyClipboard() );
			Assert::IsTrue( CloseClipboard() );
			Assert::IsFalse( EmptyClipboard() );//unerwartet, da in der hilfe steht für jeden openclipboard braucht es ein  CloseClipboard
			Assert::IsFalse( CloseClipboard() );
		}
		TEST_METHOD(UT_clipboard_default)
		{
			{
				auto openclipboard = WS::Clipboard{};
				Assert::IsFalse( openclipboard );
				Assert::IsFalse( openclipboard.is_open() );

				//auto openclipboard2 = openclipboard;//error C2280: 'WS::Clipboard::Clipboard(const WS::Clipboard &)': attempting to reference a deleted function
			}
		}
		TEST_METHOD(UT_clipboard_open)
		{
			{
				auto openclipboard = WS::Clipboard{nullptr};
				Assert::IsTrue( openclipboard );
				Assert::IsTrue( openclipboard.is_open() );

				auto x = GetOpenClipboardWindow();x;
				openclipboard.close();
				Assert::IsFalse( openclipboard );
				Assert::IsFalse( openclipboard.is_open() );
			}
		}
		TEST_METHOD(UT_clipboard_as_parameter)
		{
			{
				auto openclipboard = WS::Clipboard{nullptr};
				Assert::IsTrue( openclipboard );
				[]( WS::Clipboard && ocb )->void{ Assert::IsTrue( ocb ); }( std::move(openclipboard) );
				Assert::IsTrue( openclipboard );
				Assert::IsTrue( EmptyClipboard() );
				Assert::IsTrue( EmptyClipboard() );
			}
			Assert::IsFalse( EmptyClipboard() );
		}
		TEST_METHOD(UT_clipboard_as_parameter_return_is_open)
		{
			{
				auto openclipboard = WS::Clipboard{};
				Assert::IsFalse( openclipboard );
				[]( WS::Clipboard && ocb )->void{ ocb.open();}( std::move(openclipboard) );
				Assert::IsTrue( openclipboard );
			}
		}
		TEST_METHOD(UT_clipboard_return_is_open)
		{
			{
				auto openclipboard = []()->WS::Clipboard{ auto ocb = WS::Clipboard{nullptr}; return ocb;}();
				Assert::IsTrue( openclipboard );
				{
					Assert::IsTrue( EmptyClipboard() );
					auto openclipboard2 = WS::Clipboard{nullptr};
					Assert::IsTrue( openclipboard2 );//geht auch zweimal
					Assert::IsTrue( EmptyClipboard() );
					Assert::IsTrue( openclipboard2.close() );
					Assert::IsFalse( EmptyClipboard() );//lol
				}
				Assert::IsFalse( EmptyClipboard() );
			}
			Assert::IsFalse( EmptyClipboard() );
			auto openclipboard2 = WS::Clipboard{nullptr};
			Assert::IsTrue( openclipboard2 );
			Assert::IsTrue( EmptyClipboard() );
			Assert::IsTrue( EmptyClipboard() );
			Assert::IsTrue( openclipboard2.close() );
			Assert::IsFalse( EmptyClipboard() );
		}
	};
}

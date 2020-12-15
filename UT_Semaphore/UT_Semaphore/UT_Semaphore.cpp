#include "pch.h"
#include "CppUnitTest.h"
#include "Windows.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\..\WernersTools\headeronly\mutex_automicflag.h"

#include <atomic>
#include <mutex>
#include <condition_variable>

namespace 
{
	void out( char const * text ) 
	{
		Logger::WriteMessage(text);
	}
	void out( wchar_t const * text ) 
	{
		Logger::WriteMessage(text);
	}
}

namespace WS
{
	class Semaphore
	{
		std::mutex					pulse_mutex{};
		std::mutex					mu{};
		std::condition_variable		cv{};
		std::atomic_bool			running{false};
		std::atomic<size_t>			waiting{0};

	public:
		bool operator()() const {return running;}

		
		void wait( )//geht zuverlässig mit pulse
		{
			WS::lock_guard<decltype(pulse_mutex)> pulse_lock(pulse_mutex);

			if(running==false)
			{
				std::unique_lock<std::mutex> lk( mu );
				++waiting;
				cv.wait( lk, [this,&pulse_lock]
					{
						pulse_lock.unlock();
						return (*this)(); 
					}
				);
				--waiting;
			}
		}
		void signal(){running=true;cv.notify_all();}
		void set_running(){running=true;cv.notify_all();}
		void reset(){running=false;}
		void set_blocked(){running=false;}
		void pulse() //mit wait
		{
			const std::lock_guard<decltype(pulse_mutex)> lock(pulse_mutex);//blockiert neue wait-aufrufe und damit das hochzäh len von waiting
			set_running();
			cv.notify_all();
			while( waiting )
				std::this_thread::yield();
			set_blocked();
		}

		size_t Waiting(){return waiting;}
	};
}

namespace UTSemaphore
{
	TEST_CLASS(UTSemaphore)
	{
	public:
		
		TEST_METHOD(pulse_100)
		{
			WS::Semaphore		sema;
			bool				ready{false};
			std::atomic<size_t> counter{0};
			std::atomic<size_t>	threads_running{0};

			auto fn = [&]
			{
				for(;ready==false;)
				{
					sema.wait();//wartet bis sema in running-state  versetzt wird bzw. hier pulse ausgeführt wird. pulse soll wait-blockade genau einmal lösen. der nächste aufruf von wait blockiert bis zum nächsten pulse
					++counter;//das ist die arbeit die der thread ausführt. kurz, aber immer hin klar definiert
				}
				--threads_running;
			};
			auto start_thread = [&]
			{
				try
				{
					std::thread(fn).detach();
					++threads_running;
				}
				catch( std::exception & e)
				{
					auto str = std::string(__FUNCTION__ " ") +   e.what();str;
					//out( str.c_str() );
				}
				catch(...)
				{
					out( __FUNCTION__ " exception" );
				}
			};
			using namespace std::chrono_literals;
			Assert::IsTrue( counter==0);

			sema.pulse();
			Assert::IsTrue( counter==0);

			start_thread();
			while( sema.Waiting()!=1)
				std::this_thread::yield();

			sema.pulse();
			Assert::IsTrue( counter==1);
			counter=0;

			size_t anzahl_threads = 2000;//über 3030 kam es bei ersten tests zu "Microsoft C++ exception: std::system_error ..."

			for( size_t i=1; i<anzahl_threads; ++i )
				start_thread();

			//MessageBox(nullptr,L"wait",L"", 0);

			Assert::IsTrue( threads_running == anzahl_threads, L"es konnten nicht alle threads gestartet werden" );

			while( sema.Waiting()!=threads_running)
				std::this_thread::yield();//warten bis alle im definierten warte-zustand sind

			size_t anzahl_pulse = 1000;
			for( auto i=anzahl_pulse; i --> 0; )
			{

				sema.pulse();//funktioniert nur, wenn kein thread per therminate oder sonstige krummen dinge beendet wurde

				while( sema.Waiting()!=threads_running)
					std::this_thread::yield();//warten bis alle wieder im definierten warte-zustand sind

				Assert::IsTrue( counter==threads_running );
				counter=0;
			}

			//aufräumen, alle threads beenden lassen
			ready=true;
			sema.set_running();
			while( threads_running )
				std::this_thread::yield();
		}
	};
}

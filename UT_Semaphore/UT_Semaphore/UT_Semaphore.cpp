#include "pch.h"
#include "CppUnitTest.h"
//#include "Windows.h"

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

	//#pragma warning(suppress:4455)
	//short operator"" sh(unsigned long long int v) noexcept {return short(v);}
}

namespace WS
{
	class Semaphore
	{
		std::mutex					wait_mutex{};
		std::condition_variable		cv{};
		std::atomic_bool			running{false};
		std::atomic<size_t>			waiting{0};//anzahl wartender threads, für pulse nötig

		mutable std::mutex			state_mutex{};//dient hauptsächlich der vermeidung von race-condition
		auto _lock_state_mutex() const
		{
			return WS::lock_guard<decltype(this->state_mutex)>(this->state_mutex);
		}

	public:
		bool is_running() const {auto locked=_lock_state_mutex();return _is_running();}
		bool operator()() const {return is_running();}
		operator bool () const {return is_running();}

		
		void wait( )//es wird ggf gewartet, bis im die Semaphore im running-mode ist
		{
			auto pulse_lock =_lock_state_mutex();
			_wait( pulse_lock );
		}

		//blocking methoden
		void reset()		{auto lock=_lock_state_mutex();_set_blocked();}
		void set_blocked()	{auto lock=_lock_state_mutex();_set_blocked();}
		void set_blocked_and_wait()
		{
			auto lock=_lock_state_mutex();
			_set_blocked();
			_wait( lock );
		}

		//running methoden
		void signaled()		{auto lock=_lock_state_mutex();_set_running();}
		void set_running()	{auto lock=_lock_state_mutex();_set_running();}
		void pulse() //jeder wartender thread soll gestartet werden, der nächste wait im thread blockiert wieder. also definiert einmalige ausführung.
		{
			auto pulse_lock=_lock_state_mutex();//blockiert neue wait-aufrufe und damit das hochzählen von waiting
			if( _is_running()==false )
			{
				_set_running();
				size_t counter = 0;size_t const threads = waiting;//basteln, weil notify_all manchmal nicht die gewünschte wirkung erzielt
				while( waiting )
				{
					if( ++counter > threads )
					{
						cv.notify_all();//???geht doch manchmal einer verloren, also nochmal anstoßen!!!
						counter=0;
					}
					std::this_thread::yield();//nicht nötig, schadet aber auch nicht
				}
				_set_blocked();
			}
		}

		size_t	Waiting(){auto lock=_lock_state_mutex();return waiting;}//anzahl der wartenden threads
		void	notify_all(){ cv.notify_all(); };//sollten wartende threads nicht gestartet werden, kann mit notify_all() der erneutet anstoß ausgelöst werden. k.A. warum das manchmal nötig ist
	private:
		//funktionen ohne eigenen lock sind private
		bool _is_running() const {return running;}
		void _set_running(){running=true;cv.notify_all();}
		void _set_blocked(){running=false;}
		void _wait( WS::lock_guard<decltype(state_mutex)> & pulse_lock)
		{
			if(running==false)
			{
				auto condition = [this,pulse_lock=std::move(pulse_lock)]() mutable //ohne mutable lambda klappt das mit dem verschieben des pulse_lock nicht, da die capture-parameter const wären
				{
					pulse_lock.unlock();//setzt beim ersten aufruf der check_funktion den mutext zurück
					return this->_is_running(); 
				};
				std::unique_lock<std::mutex> lk( wait_mutex );
				++waiting;
				cv.wait( lk, std::ref(condition) );
				--waiting;
			}
		}
	};
}

namespace UTSemaphore
{
	TEST_CLASS(UTSemaphore)
	{
	public:
		
		TEST_METHOD(_1000_pulse_auf_2000_threads)
		{
			WS::Semaphore		sema;
			bool				ready{false};
			std::atomic<size_t> counter{0};
			std::atomic<size_t>	threads_running{0};

			size_t anzahl_pulse		= 1000;
			size_t anzahl_threads	= 2000;//über 3030 threads kam es bei ersten tests zu "Microsoft C++ exception: std::system_error ..."

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


			for( size_t i=1; i<anzahl_threads; ++i )
				start_thread();

			//MessageBox(nullptr,L"wait",L"", 0);

			Assert::IsTrue( threads_running == anzahl_threads, L"es konnten nicht alle threads gestartet werden" );

			while( sema.Waiting()!=threads_running)
				std::this_thread::yield();//warten bis alle im definierten warte-zustand sind

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
		TEST_METHOD( lock_at_100_mal_20000 )
		{
			WS::Semaphore		sema;
			bool				ready{false};
			std::atomic<size_t>	counter{0};

			sema.set_running();
			auto threadfunction = [&]
			{
				for(;ready==false;)
				{
					if( ++counter == 100 )
					{
						sema.set_blocked_and_wait();//wichtig statt set_blocked();wait(); was zur race-condition führt
					}
					if( counter >= 100 )
						throw std::runtime_error(__FUNCTION__ " counter sollte wieder bei 0 sein");//ohne try catch abort
				}
			};
			auto t1 = std::thread(threadfunction);
			auto releasethreadfunction = [&]
			{
				size_t counter_inner{20000};
				while( ready==false )
				{
					if( sema.is_running()==false )
					{
						counter=0;
						if( --counter_inner == 0 )
							ready=true;//threads beenden

						//Assert::IsTrue( sema.Waiting()==1);Der aktive Testlauf wurde abgebrochen. Grund: Der Testhostprozess ist abgestürzt.
						if( sema.Waiting() != 1)
							throw std::runtime_error(__FUNCTION__ " muss hier 1 liefern");

						sema.set_running();
					}
					else
					{
						using namespace std::chrono_literals;
						//std::this_thread::sleep_for(30us);//???mit 1 us oder yield gehen scheinbar die notify_all verloren???
						//std::this_thread::yield();
						if(01)//notify_all sollte nicht nötig sein. so ein mist. Hat doch set_running() schon einmal gemacht
							sema.notify_all();//eine notification geht scheinbar schonmal verloren, wenn man lange genug wartet werden von system notifications ausgelöst. ist aber lästig
					}
				}
			};
			auto t2 = std::thread(releasethreadfunction);

			t1.join();
			t2.join();
		}
		TEST_METHOD( inc_100_mal_20000 )
		{
			for( std::atomic<size_t>	counter{0}; counter<100*20000; )
			{
				++counter;
			}
		}
		TEST_METHOD( move_lock )
		{
			std::mutex	mutex{};
			auto lock = WS::lock_guard<decltype(mutex)>(mutex);
			Assert::IsTrue( lock.is_locked() );
			decltype(lock) lock2 = std::move(lock);
			Assert::IsFalse( lock.is_locked() );
			Assert::IsTrue( lock2.is_locked() );
		}
		TEST_METHOD( std__reference_wrapper2 )
		{
			{
				int a = 3;
				int b = 4;
				auto refa = std::ref(a);
				auto refb = std::ref(b);
				Assert::IsTrue(refa < refb);
			}
			{
				struct A
				{
					int v;
					bool operator<( A const & r) const {return v<r.v;}
					//friend bool operator<( std::reference_wrapper<A> const & l, A const & r) {return l.get().v<r.v;}
				};
				A a { 3 };
				A b { 4 }; 
				auto refa = std::ref(a);
				auto refb = std::ref(b);
				//Assert::IsTrue(refa < refb);//error C2678: binary '<': no operator found which takes a left-hand operand of type 'std::reference_wrapper<UTSemaphore::UTSemaphore::std__reference_wrapper2::A>' (or there is no acceptable conversion)
				Assert::IsTrue(refa.get() < refb);
			}
		}
		TEST_METHOD( std__reference_wrapper1 )
		{
			struct A
			{
				int foo(){return 5;}
			};

			A a;
			Assert::IsTrue( a.foo() == 5 );
			std::reference_wrapper<A> ra = a;
			Assert::IsTrue( ra.get().foo() == 5 );
		}
	};
}

#pragma once

#include <optional>//WS::ICombiner
#include <tuple>
#include <functional>
#include <map>
#include <deque>
#include <atomic>
#include <mutex>

namespace WS
{
	template<typename signatur>struct template_signatur;
	//template<typename return_t, typename ... parameter_t>struct template_signatur<return_t(parameter_t...)>
	template<typename return_t, typename ... parameter>struct template_signatur<return_t(parameter...)>
	{
		using signatur_t=return_t(parameter...);
		using return_type = return_t;
		using tuple_t = std::tuple<parameter...>;
		//using parameter_types2 = parameter_t ...;
		//using signatur = return_t(parameter_t...);
	};
}
namespace WS
{
	template<typename T> struct ICombiner
	{
		using type = T;
		virtual ~ICombiner(){}
		virtual type operator()( T && value ) = 0;
		virtual type operator()( std::optional<T> & value1, T && value2 ) = 0;
	};
	template<> struct ICombiner<void>
	{
		using type = void;
		virtual ~ICombiner(){}
		virtual type operator()(void) = 0;
	};

	template<typename T> struct CombinerLastvalue : ICombiner<T>
	{
		virtual type operator()( T && value ) override { return std::move(value); }
		virtual type operator()( std::optional<T> & value1, T && value2 ) override { return std::move(value2); }
	};
	template<> struct CombinerLastvalue<void> : ICombiner<void>
	{
		virtual type operator()(void) override { }
	};

	struct SignalConnectionGuard//TODO tut noch nichts
	{
		~SignalConnectionGuard(){}
		SignalConnectionGuard(){}
		SignalConnectionGuard(SignalConnectionGuard const &) = delete;
		SignalConnectionGuard(SignalConnectionGuard &&){}
		SignalConnectionGuard& operator=(SignalConnectionGuard const &) = delete;
		SignalConnectionGuard& operator=(SignalConnectionGuard &&){return *this;}
	};
	//template<typename signatur,typename combiner=CombinerLastvalue<template_signatur<signatur>::return_type>> class ISignal : template_signatur<signatur>
	template<typename signatur> class ISignal : template_signatur<signatur>
	{
		template<typename U>
		SignalConnectionGuard connect( U u );
		void operator()();
	};
}

namespace WS
{
	class mutex_atomicflag
	{
		std::atomic_flag flag{};
	public:
		void lock(){ while (flag.test_and_set()){} }
		void unlock(){ flag.clear(); }
	};
}
namespace WS
{
	template<typename signatur>struct Signal_trait;
	template<typename return_type, typename ... parameter_types>struct Signal_trait<return_type(parameter_types...)>
	{
		using signatur_t		= return_type(parameter_types...);
		using return_t          = return_type;
		using tuple_t			= std::tuple<parameter_types...>;
		using fn_t              = std::function<return_type(parameter_types...)>;
		using id_t				= size_t;
	};

	#pragma region Standard Combiner
		//Combiner für Signal
		//Combiner verarbeiten die Rückgabewerte der gerufenen Funktionen
		template<typename T,std::conditional_t<std::is_same<void,T>::value==false, T, int> initvalue=std::conditional_t<std::is_same<void,T>::value==false, T, int>{}> struct combiner_last 
		{	//combiner_last liefert in this->value den rückgabewert der letzten gerufenen Funktion, oder, wenn kein Funktion verknüpft ist, den initvalue
			static constexpr T init_val{initvalue};
			using id_t = typename Signal_trait<T()>::id_t;
			T		value{initvalue};
			id_t	last_signal_handler_id{};
			auto operator()(){return this->value;}
			auto operator()(T const & v_in,id_t id={})&{this->last_signal_handler_id=id;this->value = v_in;return *this;}
			auto operator()(T const & v_in,id_t id={}) && { return operator()(v_in,id); }
			operator T const &(){return value;}
		};
		template<> struct combiner_last<void,0> 
		{	//spezialisierung rückgabe void kann nicht gesammelt werden, weil funktionen kann kein void-parameter mit namen übergeben werden
		};
		template<typename T> struct combiner_and
		{	//combiner_and liefert alle rückgabewerte mit operator& verknüpft in std::optional<T> value
			using id_t = typename Signal_trait<T()>::id_t;
			std::optional<T> value;
			auto operator()(){return this->value;}
			auto& operator()(T const & v_in,id_t  ={}) &
			{
				if(this->value.has_value())
					this->value.value() &= v_in;
				else 
					this->value = v_in; 
				return *this; 
			}
			auto operator()(T const & v_in,id_t id={}) && { return operator()(v_in,id); }
		};
		template<typename T> struct combiner_or
		{	//combiner_and liefert alle rückgabewerte mit operator| verknüpft in std::optional<T> value
			using id_t = typename Signal_trait<T()>::id_t;
			std::optional<T> value;
			auto operator()(){return this->value;}
			auto& operator()(T const & v_in,id_t id={}) & 
			{
				if(this->value.has_value())
					this->value.value() |= v_in;
				else 
					this->value = v_in; 
				return *this; 
			}
			auto  operator()(T const & v_in,id_t id={}) && { return operator()(v_in,id); }
		};
		template<typename T> struct combiner_all
		{	//combiner_and liefert alle rückgabewerte in std::deque<std::pair<T,id_t>> value. der id_t ist die id, die beim Signal.connect im Connection_Guard geliefert wurde
			using id_t = typename Signal_trait<T()>::id_t;
			std::deque<std::pair<T,id_t>> value;
			auto operator()(){return this->value;}
			auto& operator()(T const & v_in,id_t id={}) & 
			{
				this->value.push_back({v_in,id});
				return *this; 
			}
			auto  operator()(T const & v_in,id_t id={}) && { return operator()(v_in,id); }
		};
	#pragma endregion

	
	template<typename signatur,typename combiner_t=combiner_last<Signal_trait<signatur>::return_t>>struct Signal;
	template<typename return_type, typename ... parameter_types,typename combiner_type>struct Signal<return_type(parameter_types...),typename combiner_type> : Signal_trait<return_type(parameter_types...)>
	{
		using combiner_t=combiner_type;
		struct Connection_Guard 
		{	//Connection_Guard entfernt automatisch im dtor die callback-funktion aus der Signal-Funktionspointer-Verwaltung
			using Signal_t=Signal<return_type(parameter_types...),combiner_t>;
			id_t					id		= 0;
			Signal_t *				signal	= nullptr;
			~Connection_Guard(){disconnect();}
			Connection_Guard(){}
			Connection_Guard(Connection_Guard const &) = delete;
			Connection_Guard(Connection_Guard && r) {swap(r);}
			Connection_Guard& operator=(Connection_Guard const &)& = delete;
			Connection_Guard& operator=(Connection_Guard && r )&{Connection_Guard{std::move(r)}.swap(*this);return *this;}

			Connection_Guard(Signal_t* signal, id_t id) : signal(signal), id(id){}

			void swap(Connection_Guard & r)
			{
				std::swap(this->id, r.id);
				std::swap(this->signal, r.signal);
			}

			[[nodiscard]]
			id_t release(){signal=nullptr;return id;}//der auf aufrufer verantwortet den disconnect selbst
			void disconnect();
		};
		std::map<id_t,fn_t> callbacks;//Funktionspointer-Verwaltung

		Signal( ) {}
		template<typename fn_in_t> 
		[[nodiscard]]
		Connection_Guard connect( fn_in_t fn ) //für this->operator()(...) objekte evtl connect(std::reference_wrapper(*this)) verwenden, da 'fn' immer kopiert wird
		{ 
			std::lock_guard<decltype(locker)> const lock{locker};
			id_t id = ++last_id; 
			callbacks[id] = fn;
			return {this,id};
		}
		auto operator()( parameter_types... args)
		{
			std::lock_guard<decltype(locker)> const lock{locker};
			if constexpr(std::is_same<return_t,void>::value==false)
			{
				combiner_type combiner{};
				for(auto &[id,fn] : this->callbacks)
					combiner( fn(args...), id );
					//return_value = fn(std::forward<parameter_types>(args)...);
				return combiner;
			}
			else for(auto & [id,fn] : this->callbacks)
				fn(args...);
		}
		void disconnect( id_t id )
		{
			if( id )
			{
				std::lock_guard<decltype(locker)> const lock{locker};
				callbacks.erase(id);
			}
		}
	private:
		std::atomic<id_t>	last_id = 0;
		mutex_atomicflag	locker {};	
	};

	template<typename ret_t,typename...args,typename combiner_t> void Signal<ret_t(args...),combiner_t>::Connection_Guard::disconnect( )
	{
		if( this->signal )
		{
			this->signal->disconnect(this->id);
			this->signal=nullptr;
		}
	}
}
#pragma once

#include <optional>//WS::ICombiner
#include <tuple>
#include <functional>
#include <map>
#include <atomic>

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
	template<typename signatur>struct Signal;
	template<typename signatur>struct Signal_trait;
	template<typename return_type, typename ... parameter_types>struct Signal_trait<return_type(parameter_types...)>
	{
		using signatur_t		= return_type(parameter_types...);
		using return_t          = return_type;
		using tuple_t			= std::tuple<parameter_types...>;
		using fn_t              = std::function<return_type(parameter_types...)>;
		using id_t				= size_t;
	};

	
	template<typename return_type, typename ... parameter_types>struct Signal<return_type(parameter_types...)> : Signal_trait<return_type(parameter_types...)>
	{
		struct Connection_Guard 
		{
			id_t					id		= 0;
			Signal<signatur_t> *	signal	= nullptr;
			Connection_Guard(){}
			Connection_Guard(Signal<return_type(parameter_types...)>* signal, id_t id) : signal(signal), id(id){}
			~Connection_Guard();
		};
		std::map<id_t,fn_t> callbacks;

		Signal( ) {}
		template<typename fn_in_t> 
		Connection_Guard connect( fn_in_t fn ) 
		{ 
			while(this->lock.test_and_set()){}
			id_t id = ++last_id; 
			callbacks[id] = fn;
			this->lock.clear();
			return {this,id};
		}
		return_t operator()( parameter_types... args)
		{
			while(this->lock.test_and_set()){}
			if constexpr(std::is_same<return_t,void>::value==false)
			{
				return_t return_value{};
				for(auto &[id,fn] : this->callbacks)
					return_value = fn(args...);
					//return_value = fn(std::forward<parameter_types>(args)...);
				this->lock.clear();
				return return_value;
			}
			else for(auto & [id,fn] : this->callbacks)
				fn(args...);

			this->lock.clear();
		}
		void disconnect( Connection_Guard & connection )
		{
			if( connection.id )
			{
				while(this->lock.test_and_set()){}
				callbacks.erase(connection.id);
				this->lock.clear();
			}
		}
	private:
		std::atomic<id_t>	last_id = 0;
		std::atomic_flag	lock {};	
	};
	template<typename signatur, typename fn_type> Signal<typename signatur> make_Signal( fn_type fn )
	{
		return Signal<signatur,std::decay<fn_type>>{std::move(fn)};
	}

	template<typename ret_t,typename...args> Signal<ret_t(args...)>::Connection_Guard::~Connection_Guard()
	{
		if( this->signal )
		{
			this->signal->disconnect(*this);
			this->signal=nullptr;
		}
	}
}
#pragma once

#include <optional>//WS::ICombiner
#include <tuple>
#include <functional>
#include <map>
#include <deque>
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
	template<typename T,std::conditional_t<std::is_same<void,T>::value==false, T, int> initvalue=std::conditional_t<std::is_same<void,T>::value==false, T, int>{}> struct combiner_last 
	{	
		static constexpr T init_val{initvalue};
		T value{initvalue};
		auto operator()(){return this->value;}
		auto operator()(T const & v_in)&{this->value = v_in;return *this;}
		auto operator()(T const & v_in) && { return operator()(v_in); }
		operator T const &(){return value;}
	};
	template<> struct combiner_last<void,0> 
	{	
	};
	template<typename T> struct combiner_and
	{
		std::optional<T> value;
		auto operator()(){return this->value;}
		auto& operator()(T const & v_in) &
		{
			if(this->value.has_value())
				this->value.value() &= v_in;
			else 
				this->value = v_in; 
			return *this; 
		}
		auto operator()(T const & v_in) && { return operator()(v_in); }
		operator T const &(){return value;}
	};
	template<typename T> struct combiner_or
	{
		std::optional<T> value;
		auto operator()(){return this->value;}
		auto& operator()(T const & v_in) &
		{
			if(this->value.has_value())
				this->value.value() |= v_in;
			else 
				this->value = v_in; 
			return *this; 
		}
		auto operator()(T const & v_in) && { return operator()(v_in); }


		operator T const &(){return value;}
	};
	template<typename T> struct combiner_all
	{
		std::deque<T> value;
		auto operator()(){return this->value;}
		auto& operator()(T const & v_in) &
		{
			this->value.push_back(v_in);
			return *this; 
		}
		auto operator()(T const & v_in) && { return operator()(v_in); }


		operator T const &(){return value;}
	};

	template<typename signatur>struct Signal_trait;
	template<typename return_type, typename ... parameter_types>struct Signal_trait<return_type(parameter_types...)>
	{
		using signatur_t		= return_type(parameter_types...);
		using return_t          = return_type;
		using tuple_t			= std::tuple<parameter_types...>;
		using fn_t              = std::function<return_type(parameter_types...)>;
		using id_t				= size_t;
	};

	
	template<typename signatur,typename combiner_t=combiner_last<Signal_trait<signatur>::return_t>>struct Signal;
	template<typename return_type, typename ... parameter_types,typename combiner_type>struct Signal<return_type(parameter_types...),typename combiner_type> : Signal_trait<return_type(parameter_types...)>
	{
		using combiner_t=combiner_type;
		struct Connection_Guard 
		{
			using Signal_t=Signal<return_type(parameter_types...),combiner_t>;
			id_t					id		= 0;
			Signal_t *				signal	= nullptr;
			Connection_Guard(){}
			Connection_Guard(Signal_t* signal, id_t id) : signal(signal), id(id){}
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
		auto operator()( parameter_types... args)
		{
			while(this->lock.test_and_set()){}
			if constexpr(std::is_same<return_t,void>::value==false)
			{
				combiner_type combiner{};
				for(auto &[id,fn] : this->callbacks)
					combiner( fn(args...) );
					//return_value = fn(std::forward<parameter_types>(args)...);
				this->lock.clear();
				return combiner();
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
	template<typename signatur, typename fn_type,typename combiner_t > Signal<typename signatur,combiner_t> make_Signal( fn_type fn )
	{
		return Signal<signatur,std::decay<fn_type>>{std::move(fn)};
	}

	template<typename ret_t,typename...args,typename combiner_t> Signal<ret_t(args...),combiner_t>::Connection_Guard::~Connection_Guard()
	{
		if( this->signal )
		{
			this->signal->disconnect(*this);
			this->signal=nullptr;
		}
	}
}
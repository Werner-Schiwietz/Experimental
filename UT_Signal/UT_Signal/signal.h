#pragma once

#include <optional>//WS::ICombiner
#include <tuple>

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
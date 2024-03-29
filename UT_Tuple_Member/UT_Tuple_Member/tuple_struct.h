#pragma once

#include <tuple>

template<typename enum_type, typename ... types>
struct tuple_struct
{	
	using typelist = WS::typelist<types...>;
	using tuple_t = typename typelist::tuple_t;
	using member = enum_type;
	tuple_t values{};

	//template<member index> auto & operator()() &		{return std::get<index>(this->values);}				//operator() funktioniert, aber bringt f�r den aufrufer nichts, nur unverst�ndnis. aufruf per v.[template ]operator()<0>(); [template ] optinal evtl. bei einigen compilern n�tig
	//template<member index> auto   operator()()  &&		{return std::move(std::get<index>(this->values));}	//operator() funktioniert, aber bringt f�r den aufrufer nichts, nur unverst�ndnis
	template<member index> auto & access() &			
	{
		return std::get<std::underlying_type_t<member>(index)>(this->values);
	}
	template<member index> auto   access() &&			
	{
		return std::move(std::get<std::underlying_type_t<member>(index)>(this->values));
	}

	template<member index,typename T> tuple_struct& set(T&&value) &	
	{
		std::get<std::underlying_type_t<member>(index)>(this->values) = std::forward<T>(value);
		return *this;
	}
	template<member index,typename T> tuple_struct set(T&&value) &&
	{
		std::get<std::underlying_type_t<member>(index)>(this->values) = std::forward<T>(value);
		return std::move(*this);
	}

	template<typename io_interface> void WriteData( io_interface && io ) const
	{
		::WriteData( std::forward<io_interface>(io), this->values );
	}
	template<typename io_interface> void ReadData( io_interface && io )
	{
		::ReadData( std::forward<io_interface>(io), this->values );
	}
};

//template<typename io_interface, typename enum_type,typename ... types> void WriteData( io_interface && io, tuple_struct<enum_type,types...> const & value  )
//{
//	WriteData( std::forward<io_interface>(io), value.values );
//}
//template<typename io_interface, typename enum_type,typename ... types> void ReadData( io_interface && io, tuple_struct<enum_type,types...> & value  )
//{
//	ReadData( std::forward<io_interface>(io), value.values );
//}



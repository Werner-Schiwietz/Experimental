#pragma once
//Copyright (c) 2021 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

#include <memory>

#pragma region interface-vorlagen fuer ReadData WriteData
template<typename return_type> struct Idata_output
{
	virtual ~Idata_output(){}
	virtual return_type Write( void const *,size_t bytes) = 0;
};
template<typename return_type> struct Idata_input
{
	virtual ~Idata_input(){}
	virtual return_type Read( void *,size_t bytes) = 0;
};
#pragma endregion

#pragma region lowlevel ReadWrite
//SFINAE: Substitution Failure Is Not An Error
template <typename T, typename io_interface> auto hasmethod_ReadData(unsigned long) -> std::false_type;
template <typename T, typename io_interface> auto hasmethod_ReadData(int) -> decltype( std::declval<T>().ReadData(std::declval<io_interface>()), std::true_type{} );
template <typename T, typename io_interface> auto hasmethod_WriteData(unsigned long) -> std::false_type;
template <typename T, typename io_interface> auto hasmethod_WriteData(int) -> decltype( std::declval<T>().WriteData(std::declval<io_interface>()), std::true_type{} );
template <typename T, typename io_interface> auto hasmethod_Load(unsigned long) -> std::false_type;
template <typename T, typename io_interface> auto hasmethod_Load(int) -> decltype( std::declval<T>().Load(std::declval<io_interface>()), std::true_type{} );
template <typename T, typename io_interface> auto hasmethod_Save(unsigned long) -> std::false_type;
template <typename T, typename io_interface> auto hasmethod_Save(int) -> decltype( std::declval<T>().Save(std::declval<io_interface>()), std::true_type{} );

template <typename T, typename io_interface> auto has_Load_ctor(unsigned long) -> std::false_type;
template <typename T, typename io_interface> auto has_Load_ctor(int) -> decltype( T{std::declval<io_interface>()}, std::true_type{} );



template<typename io_interface>auto ReadData( io_interface && io, void * value, size_t bytes ) -> decltype( std::forward<io_interface>(io).Read( value, bytes ) )
{
	if constexpr ( std::is_integral_v<decltype( std::forward<io_interface>(io).Read( value, bytes ) )> )
	{
		struct ReadData_data_missing : std::exception{using std::exception::exception;};
		auto read = std::forward<io_interface>(io).Read( value, bytes );
		if( read != bytes )
			throw ReadData_data_missing{ __FUNCTION__ " zu wenig Daten gelesen" };
		return read;
	}
	else
		return std::forward<io_interface>(io).Read( value, bytes );
}
template<typename io_interface> auto ReadData( io_interface && io, void* value, size_t bytes ) -> decltype( ReadData( (*std::forward<io_interface>(io)),value,bytes) )
{
	return ReadData( (*std::forward<io_interface>(io)),value,bytes);
}

template<typename io_interface>auto WriteData( io_interface && io, void const * value, size_t bytes ) -> decltype( std::forward<io_interface>(io).WriteData((void const*)nullptr,(size_t)0) )
{
	return std::forward<io_interface>(io).WriteData( value, bytes );
}
template<typename io_interface>auto WriteData( io_interface && io, void const * value, size_t bytes ) -> decltype( std::forward<io_interface>(io).Write(value, bytes) )
{
	return std::forward<io_interface>(io).Write( value, bytes );
}
template<typename io_interface>auto WriteData( io_interface && io, void const * value, size_t bytes ) -> decltype( std::forward<io_interface>(io)->Write(value, bytes) )
{
	return std::forward<io_interface>(io)->Write( value, bytes );
}

#pragma endregion

template<typename io_interface, typename T>	auto ReadData( io_interface && io, T & value  )
{
	if constexpr ( decltype(hasmethod_ReadData<T,io_interface&&>(0))::value )
	{
		return value.ReadData( std::forward<io_interface>( io ) );
	}
	else if constexpr ( decltype(hasmethod_Load<T,io_interface&&>(0))::value )
	{
		return value.Load( std::forward<io_interface>( io ) );
	}
	else
	{
		static_assert(decltype(has_Load_ctor<T,io_interface&&>(0))::value == false, "use ReadData<T>(io)" );
		static_assert(std::is_pointer_v<T> == false, "is pointer!!. saving address???");
		return ReadData( std::forward<io_interface>( io ), (void *)&value, sizeof( value ) );
	}
}
template<typename T,typename io_interface>	T ReadData( io_interface && io )
{
	if constexpr ( decltype(hasmethod_Load<T,io_interface&&>(0))::value )
	{
		return T{std::forward<io_interface>( io )};
	}
	else
	{
		T value{};
		ReadData( std::forward<io_interface>( io ), value );
		return value;
	}
}

template<typename io_interface, typename T> auto WriteData( io_interface && io, T const & value  )
{
	if constexpr ( decltype(hasmethod_WriteData<T const,io_interface&&>(0))::value )
	{
		return value.WriteData( std::forward<io_interface>( io ) );
	}
	else if constexpr ( decltype(hasmethod_Save<T const,io_interface&&>(0))::value )
	{
		return value.Save( std::forward<io_interface>( io ) );
	}
	else
	{
		static_assert(std::is_pointer_v<T> == false);
		return WriteData( std::forward<io_interface>( io ), (void const *)&value, sizeof( value ) );
	}
}

#pragma region variadic 
template<typename io_interface, typename T, typename ... Ts> auto ReadDatas( io_interface && io, T & value, Ts & ... rest  )
{
	ReadData( std::forward<io_interface>(io), value );
	ReadData( std::forward<io_interface>(io), rest... );
}
template<typename io_interface, typename T, typename ... Ts> auto WriteDatas( io_interface && io, T const & value, Ts const &  ... rest  )
{
	WriteData( std::forward<io_interface>(io), value );
	WriteData( std::forward<io_interface>(io), rest... );
}
#pragma endregion 

#pragma region smart-pointer
template<typename io_interface, typename ptr_t>	void _WriteData( io_interface && io, ptr_t const & value  )
{
	WriteData( std::forward<io_interface>(io), value!=nullptr );
	if( value )
	{
		(void)WriteData( std::forward<io_interface>(io), *value );
	}
}

template<typename io_interface, typename T>	void WriteData( io_interface && io, std::unique_ptr<T> const & value )
{
	static_assert( std::is_array_v<T> == false );//array kann nicht funktionieren, da die anzahl der arrayitems unbekannt ist
	_WriteData( std::forward<io_interface>(io), value );
}
template<typename io_interface, typename T>	void ReadData( io_interface && io, std::unique_ptr<T> & value )
{
	static_assert( std::is_array_v<T> == false );//array kann nicht funktionieren, da die anzahl der arrayitems unbekannt ist

	if( ReadData<bool>(std::forward<io_interface>(io)) )
	{
		if constexpr ( decltype(has_Load_ctor<T,io_interface&&>(0))::value )
		{
			value = std::make_unique<T>( std::forward<io_interface>( io ) );
		}
		else
		{
			value = std::make_unique<T>();
			(void)ReadData( std::forward<io_interface>( io ), *value );
		}
	}
}
template<typename io_interface, typename T>	void WriteData( io_interface && io, std::shared_ptr<T> const & value )
{
	static_assert( std::is_array_v<T> == false );
	_WriteData( std::forward<io_interface>(io), value );
}
template<typename io_interface, typename T>	void ReadData( io_interface && io, std::shared_ptr<T> & value )
{
	//std::unique_ptr<T> v;
	//ReadData( std::forward<io_interface>(io), v );
	//value = std::shared_ptr<T>{std::move(v)};
	value = std::shared_ptr<T>{ ReadData<std::unique_ptr<T>>(std::forward<io_interface>(io)) };
}

#pragma region WS::auto_ptr 
namespace WS
{
	template<typename T> class auto_ptr;
}
template<typename io_interface, typename T>	void WriteData( io_interface && io, WS::auto_ptr<T> const & value )
{
	static_assert( std::is_array_v<T> == false );
	_WriteData( std::forward<io_interface>(io), value );
}
template<typename io_interface, typename T>	void ReadData( io_interface && io, WS::auto_ptr<T> & value )
{
	value = WS::auto_ptr<T>{ ReadData<std::unique_ptr<T>>(std::forward<io_interface>(io)) };
}
#pragma endregion 
#pragma endregion 

#pragma region tuple
template<typename io_interface, typename ... types> void WriteData( io_interface && io, std::tuple<types...> const & value )
{
	auto write = [&](auto const & ... values)
	{
		WriteDatas( std::forward<io_interface>(io), values ... );
	};
	std::apply(write,value);//c++17
}
template<typename io_interface, typename ... types> void ReadData( io_interface && io, std::tuple<types...> & value )
{
	auto read = [&](auto & ... values)
	{
		ReadDatas( std::forward<io_interface>(io), values ... );
	};
	std::apply(read,value);//c++17
}
#pragma endregion 

#pragma region pair
template<typename io_interface, typename first_t, typename second_t> void WriteData( io_interface && io, std::pair<first_t,second_t> const & value )
{
	WriteData( std::forward<io_interface>(io), value.first );
	WriteData( std::forward<io_interface>(io), value.second );
}
template<typename io_interface, typename first_t, typename second_t> void ReadData( io_interface && io, std::pair<first_t,second_t> & value )
{
	value = 
	{ 
		ReadData<first_t>(  std::forward<io_interface>(io) ),
		ReadData<second_t>( std::forward<io_interface>(io) )
	};
}
#pragma endregion 



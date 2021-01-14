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
struct Idata_output
{
	virtual ~Idata_output(){}
	virtual void WriteData( void const *,size_t bytes) = 0;
};
struct Idata_input
{
	virtual ~Idata_input(){}
	virtual void ReadData( void *,size_t bytes) = 0;
};
#pragma endregion

#pragma region lowlevel ReadWrite
template<typename io_interface>auto ReadData( io_interface && io, void * value, size_t bytes ) -> decltype( std::forward<io_interface>(io).ReadData( value, bytes ) )
{
	return std::forward<io_interface>(io).ReadData( value, bytes );
}
template<typename io_interface> auto ReadData( io_interface && io, void* value, size_t bytes ) -> decltype( std::forward<io_interface>(io).Read(value,bytes) )
{
	return std::forward<io_interface>(io).Read( value, bytes );
}
template<typename io_interface> auto ReadData( io_interface && io, void* value, size_t bytes ) -> decltype( std::forward<io_interface>(io)->Read(value,bytes) )
{
	return std::forward<io_interface>(io)->Read( value, bytes );
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
	static_assert( std::is_pointer_v<T> == false );
	return ReadData( std::forward<io_interface>(io), (void*)&value, sizeof(value) );
}
template<typename T,typename io_interface>	T ReadData( io_interface && io ){T value{}; ReadData(std::forward<io_interface>(io),value); return value;}

template<typename io_interface, typename T> auto WriteData( io_interface && io, T const & value  )
{
	static_assert( std::is_pointer_v<T> == false );
	return WriteData( std::forward<io_interface>(io), (void const *)&value, sizeof(value) );
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
		value = std::make_unique<T>();
		(void)ReadData( std::forward<io_interface>(io), *value );
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




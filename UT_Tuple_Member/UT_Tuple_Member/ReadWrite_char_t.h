#pragma once
//Copyright (c) 2021 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

#include "ReadWrite.h"
#include "..\..\..\WernersTools\headeronly\char_helper.h"

#ifndef is_is_char_type_defined
	#include "..\..\..\WernersTools\headeronly\is_char_type.h" 
#endif

#pragma region char_t* spezialisierungen
template<typename io_interface,typename char_t> auto ReadData( io_interface && io, char_t * & value  ) ->std::enable_if_t<WS::is_char_type_v<char_t>,void>//value wird per delete zerst�rt und muss auch vom aufrufer ggf per delete zerst�rt qwerden. besser die version mit unique_ptr benutzen
{
	delete [] value;

	auto chars = ReadData<size_t>( io );
	if( chars == size_t(-1) )
		value=nullptr;
	else
	{
		value=new char_t[chars+1];
		
		_ReadData( std::forward<io_interface>(io), value, chars * sizeof(char_t) );
		//_ReadData( std::forward<io_interface>(io), (void*)value, chars*sizeof(char_t) );
		value[chars] = char_t{0};
	}
}
template<typename io_interface,typename char_t> auto ReadData( io_interface && io, std::unique_ptr<char_t[]> & value ) ->std::enable_if_t<WS::is_char_type_v<char_t>,void>
{
	value = std::unique_ptr<char_t[]>{ ReadData<char_t*>( std::forward<io_interface>(io) ) };
}
template<typename io_interface,typename char_t> auto WriteData( io_interface && io, std::unique_ptr<char_t[]> const & value ) ->std::enable_if_t<WS::is_char_type_v<char_t>,void>
{
	WriteData( std::forward<io_interface>(io), value.get() );
}
template<typename io_interface,typename char_t> auto WriteData( io_interface && io, char_t const * const & value ) ->std::enable_if_t<WS::is_char_type_v<char_t>,void>
{
	if( value )
	{	
		auto chars = static_cast<size_t>(stringlen(value));
		WriteData( std::forward<io_interface>(io), chars );
		_WriteData( std::forward<io_interface>(io), value, chars * sizeof(char_t) );
		//_WriteData( std::forward<io_interface>(io), value, chars * sizeof(char_t) );
	}
	else
		WriteData( std::forward<io_interface>(io), size_t(-1) );
}
template<typename io_interface,typename char_t> auto WriteData( io_interface && io, char_t * const & value  ) ->std::enable_if_t<WS::is_char_type_v<char_t>,void>
{
	return WriteData(std::forward<io_interface>(io), (char_t const *) value); 
}
#pragma endregion



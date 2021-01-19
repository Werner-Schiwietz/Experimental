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



#pragma region container

struct gibt_es_nicht{};

#pragma region std::vector std::deque und alles was clear() begin() size() und push_back(v) als methoden hat
template<typename io_interface,template<typename...>typename container_t,typename value_t, typename...others> auto ReadData( io_interface && io, container_t<value_t,others...> & container ) ->std::enable_if_t<
	std::is_same_v<gibt_es_nicht,decltype(container.clear())> == false//clear() wird in ReadData benutzt und ist aber nicht const
	&& std::is_same_v<gibt_es_nicht,decltype(container.size())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.begin())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.push_back(std::declval<value_t>()))> == false//push_back(v) wird in ReadData benutzt und ist aber nicht const
	,void>
{
	container.clear();
	for( auto anzahl=ReadData<decltype(container.size())>(std::forward<io_interface>(io)); anzahl --> 0; )
	{
		container.push_back( ReadData<value_t>(std::forward<io_interface>(io)) );
	}
}

template<typename io_interface,template<typename...>typename container_t,typename value_t, typename...others> auto ReadData( io_interface && io, container_t<value_t,others...> & container ) ->std::enable_if_t<
	std::is_same_v<gibt_es_nicht,decltype(container.clear())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.size())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.begin())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.insert(std::declval<value_t>()))> == false
	,void>
{
	container.clear();
	for( auto anzahl=ReadData<decltype(container.size())>(std::forward<io_interface>(io)); anzahl --> 0; )
	{
		container.insert( ReadData<value_t>(std::forward<io_interface>(io)) );
	}
}
template<typename io_interface,template<typename...>typename container_t,typename key_t,typename value_t, typename...others> auto ReadData( io_interface && io, container_t<key_t,value_t,others...> & container ) ->std::enable_if_t<
	std::is_same_v<gibt_es_nicht,decltype(container.clear())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.size())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.begin())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.insert(std::declval<std::pair<key_t,value_t>>()))> == false
	,void>
{
	container.clear();
	for( auto anzahl=ReadData<decltype(container.size())>(std::forward<io_interface>(io)); anzahl --> 0; )
	{
		container.insert( ReadData<std::pair<key_t,value_t>>(std::forward<io_interface>(io)) );
	}
}

template<typename io_interface,template<typename...>typename container_t,typename value_t, typename...others> auto _WriteDataContainer( io_interface && io, container_t<value_t,others...> const & container )
{
	WriteData(std::forward<io_interface>(io), container.size());
	for( auto const & item : container )
	{
		WriteData(std::forward<io_interface>(io), item);
	}
}
template<typename io_interface,template<typename...>typename container_t,typename value_t, typename...others> auto WriteData( io_interface && io, container_t<value_t,others...> const & container ) ->std::enable_if_t<
	std::is_same_v<gibt_es_nicht,decltype(container.size())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.begin())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(std::declval<std::remove_const_t<std::decay_t<decltype(container)>>>().clear())> == false//clear() wird in ReadData benutzt und ist aber nicht const
	&& std::is_same_v<gibt_es_nicht,decltype(std::declval<std::remove_const_t<std::decay_t<decltype(container)>>>().push_back(std::declval<value_t>()))> == false //push_back(v) wird in ReadData benutzt und ist aber nicht const
	,void>
{
	//std::vector std::deque
	_WriteDataContainer(std::forward<io_interface>(io), container );
}
template<typename io_interface,template<typename...>typename container_t,typename value_t, typename...others> auto WriteData( io_interface && io, container_t<value_t,others...> const & container ) ->std::enable_if_t<
	std::is_same_v<gibt_es_nicht,decltype(container.size())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.begin())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(std::declval<std::remove_const_t<std::decay_t<decltype(container)>>>().clear())> == false//clear() wird in ReadData benutzt und ist aber nicht const
	&& std::is_same_v<gibt_es_nicht,decltype(std::declval<std::remove_const_t<std::decay_t<decltype(container)>>>().insert(std::declval<value_t>()))> == false //insert(v) wird in ReadData benutzt und ist aber nicht const
	,void>
{
	//std::set
	_WriteDataContainer(std::forward<io_interface>(io), container );
}
template<typename io_interface,template<typename...>typename container_t,typename key_t, typename value_t, typename...others> auto WriteData( io_interface && io, container_t<key_t,value_t,others...> const & container ) ->std::enable_if_t<
	std::is_same_v<gibt_es_nicht,decltype(container.size())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(container.begin())> == false
	&& std::is_same_v<gibt_es_nicht,decltype(std::declval<std::remove_const_t<std::decay_t<decltype(container)>>>().clear())> == false//clear() wird in ReadData benutzt und ist aber nicht const
	&& std::is_same_v<gibt_es_nicht,decltype(std::declval<std::remove_const_t<std::decay_t<decltype(container)>>>().insert(std::declval<std::pair<key_t,value_t>>()))> == false //insert(v) wird in ReadData benutzt und ist aber nicht const
	,void>
{
	//std::map std::unordered_map std::multimap
	_WriteDataContainer(std::forward<io_interface>(io), container );
}

#pragma endregion



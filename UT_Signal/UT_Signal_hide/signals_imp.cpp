#include "pch.h"


#define EXT_CLASS
#include "Signal_imp.h"

template<typename sig>
void tcreate_imp( )
{
	WS::Signal<sig> {};
	auto op = &WS::Signal<sig>::operator();
}

void create_imp( )
{	//erzeugt benötigte template-instancen, um linker zu befriedigen
	tcreate_imp<void(int&)>();
}

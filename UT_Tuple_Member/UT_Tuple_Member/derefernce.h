#pragma once

template<typename char_t> auto equal_0_terminiert(char_t const * v1,char_t const * v2) -> std::enable_if_t<WS::is_char_type_v<char_t>,bool> //char_t muss char oder wchar_t sein
{ 
	return stringcmp(v1,v2)==0;
}
namespace dereferenced
{
	//equal sonderbehandlung 
	//	pointer und smart-pointer (std::unique_ptr,std::shared_ptr,WS::autp_ptr)
	//	für char_t const *. Diese werden als 0-terminierte strings verglichen
	template<typename T> auto equal( T const & _1, T const & _2 ) 
	{
		return _1 == _2;
	}
	template<typename T, typename equal_t> auto equal( T const & _1, T const & _2, equal_t && vergl ) 
	{
		return vergl(_1,_2);
	}

	template<typename T> auto equal( T * const & p1, T * const & p2 ) 
	{
		if( p1 == p2 )
			return true;
		if( p1 && p2 )
		{
			if constexpr ( WS::is_char_type_v<T> ) 
				return equal_0_terminiert(p1,p2);
			else 
				return equal(*p1,*p2);
		}

		return false;
	}
	template<typename T, typename equal_t> auto equal( T * p1, T * p2, equal_t && vergl ) 
	{
		if( p1 == p2 )
			return true;
		if( p1 && p2 )
			return vergl(p1,p2);

		return false;
	}

	#pragma region smart-ptr
		template<typename T> auto equal( std::unique_ptr<T> const & p1, std::unique_ptr<T> const & p2 ) 
		{
			return equal( p1.get(),  p2.get() );
		}
		template<typename T, typename equal_t> auto equal( std::unique_ptr<T> const & p1, std::unique_ptr<T> const & p2, equal_t && vergl )
		{
			return equal( p1.get(),  p2.get(), std::move(vergl) );
		}
		template<typename T> auto equal( std::shared_ptr<T> const & p1, std::shared_ptr<T> const & p2 ) 
		{
			return equal( p1.get(),  p2.get() );
		}
		template<typename T, typename equal_t> auto equal( std::shared_ptr<T> const & p1, std::shared_ptr<T> const & p2, equal_t && vergl )
		{
			return equal( p1.get(),  p2.get(), std::move(vergl) );
		}
		template<typename T> auto equal( WS::auto_ptr<T> const & p1, WS::auto_ptr<T> const & p2 ) 
		{
			return equal( p1.get(),  p2.get() );
		}
		template<typename T, typename equal_t> auto equal( WS::auto_ptr<T> const & p1, WS::auto_ptr<T> const & p2, equal_t && vergl )
		{
			return equal( p1.get(),  p2.get(), std::move(vergl) );
		}
	#pragma endregion 
}

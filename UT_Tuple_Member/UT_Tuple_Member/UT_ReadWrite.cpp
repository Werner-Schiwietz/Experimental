#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "ReadWrite_char_t.h"
#include "ReadWrite_char_t_array.h"
#include "ReadWrite_container.h"
#include "ReadWriteInterfaceFuerCFile.h"
#include "ReadWriteInterfaceFuerStreams.h"
#include "derefernce.h"

#include "..\..\..\WernersTools\headeronly\Auto_Ptr.h"
#include "..\..\..\WernersTools\headeronly\is_.h"

#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_map>

static_assert( WS::is_dereferenceable_v<char const *> ) ;
static_assert( WS::is_pointerable_v<char const *> ) ;
static_assert( WS::is_dereferenceable<std::unique_ptr<char const *>>::value ) ;
static_assert( WS::is_pointerable<std::unique_ptr<char const *>>::value ) ;
static_assert( WS::is_dereferenceable<std::unique_ptr<char *>>::value ) ;
static_assert( WS::is_pointerable<std::unique_ptr<char *>>::value ) ;
static_assert( WS::is_dereferenceable<std::unique_ptr<char []>>::value==false ) ;
static_assert( WS::is_pointerable<std::unique_ptr<char []>>::value==false ) ;

//std::remove_reference_t<decltype( *(std::unique_ptr<char>{}) )> x1;
//std::remove_reference_t<decltype( *(std::unique_ptr<char []>{}) )> x2;
//std::remove_reference_t<decltype( std::unique_ptr<char []>{}.get() )> x3;

struct xxx
{
	xxx(){}
	xxx(CFile*);
};
template<typename T>
struct try_ctor 
{
	//friend T::T();
	friend xxx::xxx();
};

struct bool_ctor
{
	short i{5};
	bool_ctor(bool){}
};
struct load_ctor
{
	short i{5};
	load_ctor(){}
	load_ctor(short i):i(i){}
	load_ctor(CFile*pFile) : i( ReadData<short>(pFile) )
	{		
	}
	load_ctor(CFile*pFile,int param1,std::string const &param2) : i( ReadData<short>(pFile) )
	{		
		param1;param2;
	}
	template<typename io_interface> void Save( io_interface&& io ) const
	{
		WriteData( std::forward<io_interface>(io), this->i );
	}
};
struct load_bool_ctor
{
	short i{5};
	load_bool_ctor(short i):i(i){}
	load_bool_ctor(bool) 
	{		
	}
	load_bool_ctor(CFile*pFile) : i( ReadData<short>(pFile) )
	{		
	}
	template<typename io_interface> void Save( io_interface&& io ) const
	{
		WriteData( std::forward<io_interface>(io), this->i );
	}
};
struct load_ctor_with_params
{
	short i{5};
	mutable int const * ptr;
	load_ctor_with_params(){};
	load_ctor_with_params(short i):i(i){}
	load_ctor_with_params(bool) 
	{		
	}
	load_ctor_with_params(CFile*pFile) 
		: i( ReadData<short>(pFile) )
	{	
	}
	load_ctor_with_params(CFile*pFile,int) 
		: i( ReadData<short>(pFile) )
	{	
	}
	load_ctor_with_params(CFile* pFile,std::unique_ptr<int> && ptr) 
		: i( ReadData<short>(pFile) )
		, ptr(ptr.get())
	{	
	}
	load_ctor_with_params(CFile* pFile,std::unique_ptr<int> & ptr) 
		: i( ReadData<short>(pFile) )
		, ptr(ptr.get())
	{	
	}
	template<typename io_interface> void Save( io_interface&& io ) const
	{
		WriteData( std::forward<io_interface>(io), this->i );
	}
};
struct load_with_params
{
	short i{5};
	mutable int const * ptr;
	load_with_params(){};
	load_with_params(short i):i(i){}
	load_with_params(bool) 
	{		
	}
	void Load(CFile*pFile) 
	{	
		 i = ReadData<short>(pFile);
	}
	void Load(CFile* pFile,std::unique_ptr<int> && ptrin) 
	{	
		this->i = ReadData<short>(pFile);
		this->ptr = ptrin.get();
	}
	void Load(CFile* pFile,std::unique_ptr<int> & ptrin) 
	{	
		this->i = ReadData<short>(pFile);
		this->ptr = ptrin.get();
	}
	template<typename io_interface> void Save( io_interface&& io ) const
	{
		WriteData( std::forward<io_interface>(io), this->i );
	}
};
struct A
{
	int									v1	= 1;
	short								v2	= 2;
	std::unique_ptr<char[]>				v3;
	std::unique_ptr<wchar_t[]>			v4;
	std::unique_ptr<unsigned short>		v5;
	std::unique_ptr<unsigned __int64>	v6;

	static_assert( WS::is_dereferenceable_v<std::unique_ptr<wchar_t>> );
	//static_assert( WS::is_dereferenceable_v<std::unique_ptr<wchar_t[]>> );
	bool operator==(A const & r) const
	{
		return this->v1 == r.v1
			&& this->v2 == r.v2
			&& dereferenced::equal(this->v3,r.v3, equal_0_terminiert<char> )
			&& dereferenced::equal(this->v4,r.v4 )//, equal_0_terminiert<wchar_t> )//geht auch automatisch
			&& dereferenced::equal(this->v5,r.v5)
			&& dereferenced::equal(this->v6,r.v6);
	}
	void Load( CFile* pFile)
	{
		ReadData(pFile,(*this).v1);
		ReadData(pFile,(*this).v2);
		ReadData(pFile,(*this).v3);
		ReadData(pFile,(*this).v4);
		ReadData(pFile,(*this).v5);
		ReadData(pFile,(*this).v6);
	}
	void Save( CFile* pFile ) const
	{
		WriteData(pFile,(*this).v1);
		WriteData(pFile,(*this).v2);
		WriteData(pFile,(*this).v3);
		WriteData(pFile,(*this).v4);
		WriteData(pFile,(*this).v5);
		WriteData(pFile,(*this).v6);
	}
};
void ReadData( ReadWrite_CFile&& io, A & value )//weil in UT_WriteDataReadData_struct_als_unique_ptr ReadData(ReadWrite_CFile(file), valueRead); statt ReadData(pFile, valueRead) steht
{
	ReadData( &io.File, value );
}
void WriteData( ReadWrite_CFile&& io, A const & value )
{
	WriteData( &io.File, value );
}
void ReadData( Read_Stream istream, A & value )
{
	ReadData(istream,value.v1);
	ReadData(istream,value.v2);
	ReadData(istream,value.v3);
	ReadData(istream,value.v4);
	ReadData(istream,value.v5);
	ReadData(istream,value.v6);
}
void WriteData( Write_Stream ostream,  A const & value )
{
	WriteData(ostream,value.v1);
	WriteData(ostream,value.v2);
	WriteData(ostream,value.v3);
	WriteData(ostream,value.v4);
	WriteData(ostream,value.v5);
	WriteData(ostream,value.v6);
}
static_assert( HasMethod_Load_v<A,CFile*> );
static_assert( HasMethod_Save_v<A const,CFile*> );

struct IMemBuf: std::streambuf
{
	IMemBuf(const char* base, size_t size)
	{
		char* p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
};

struct IMemStream: virtual IMemBuf, std::istream
{
	IMemStream(const char* mem, size_t size) :
		IMemBuf(mem, size),
		std::istream(static_cast<std::streambuf*>(this))
	{
	}
};

struct OMemBuf: std::streambuf
{
	OMemBuf(char* base, size_t size)
	{
		this->setp(base, base + size);
	}
};
struct OMemStream: virtual OMemBuf, std::ostream
{
	OMemStream(char* mem, size_t size) :
		OMemBuf(mem, size),
		std::ostream(static_cast<std::streambuf*>(this))
	{
	}
};

template<typename T> struct _adder
{
	using type=T;
	T value{};
	_adder(){}
	_adder( T const & v):value(v){}
	T add(T const & r){return this->value += r;}
	operator T() const {return value;}
};
template<> struct _adder<void>
{
	using type=void;
	_adder( void ){}
	void add( void ){}
	//void add( int={} ){}
	//template<typename ... Ts> void add(Ts...ps){}
	operator void() const {return;}
};

template<typename T> T const & foo_ret( T const & value )
{
	return value;
}
template<typename T> decltype(auto) foo( T && value ) 
{
	return foo_ret(std::forward<T>(value));
}
template<typename T> T&& foo1_ret( T && value )
{
	return std::forward<T>(value);
}
//template<typename T> auto foo1_ret( T && value ) -> std::enable_if<std::is_rvalue_reference_v<decltype(value)> == false,T const &>
//{
//	return value;
//}
template<typename T> decltype(auto) foo1( T && value ) 
{
	return foo1_ret(std::forward<T>(value));
}

namespace UT_ReadWriteData
{
	TEST_CLASS(UT_Verstaendnis)
	{
		TEST_METHOD(AddVoidReturnType)
		{
			auto return_void = []()->void{};
			auto return_in_type = [](auto value){ return value;};
				
			{
				auto v1 = _adder<decltype(return_in_type(5))>{};
				auto v2 = _adder{return_in_type(5)};//ctor bestimmt T
				v1.add( return_in_type(5) );
				v1.add( return_in_type(6) );
				v2.add( return_in_type(6) );
				Assert::IsTrue( v1==v2 );
			}
			{
				auto v = _adder<void>{};
				auto v1 = _adder<decltype(return_void())>{};
				//auto v2 = _adder{return_void()};//ctor bestimmt T. geht nicht mit void
				//v1.add( return_void() );//funktioniert nicht mit funktionen mit return_type void 
				//v1.add( return_void() );
			}
		}
		TEST_METHOD(std_forward)
		{
			{
				decltype(foo( 5 )) i1 = foo( 5 );//??ref auf 5
				Assert::IsTrue( i1 == 5 );
				decltype(foo( i1 )) i2 = foo( i1 );
				Assert::IsTrue( i2 == 5 );
				decltype(auto) p1 = foo( std::make_unique<int>( 6 ) );//das kann nicht gut gehen, p1 ist tot, also dangling
				auto && p2 = foo( p1 ); p2;
			}
			{
				decltype(auto) i1 = foo1(5);
				Assert::IsTrue( i1 == 5 );
				decltype(auto) i2 = foo1(i1);
				Assert::IsTrue( i2 == 5 );
				decltype(auto) p1 = foo1(std::make_unique<int>(6));//das kann nicht gut gehen, p1 ist tot, also dangling
				auto && p2 = foo1(p1);p2;
				auto p11 = foo1(std::make_unique<int>(6));//so geht es, die refenenz ist weg, das wird gemoved
				auto && p22 = foo1(p11);p22;
				//auto p23 = foo1(p11);p23;//error C2280: 'std::unique_ptr<int,std::default_delete<int>>::unique_ptr(const std::unique_ptr<int,std::default_delete<int>> &)': attempting to reference a deleted function
				foo1(p11);//error C2280: 'std::unique_ptr<int,std::default_delete<int>>::unique_ptr(const std::unique_ptr<int,std::default_delete<int>> &)': attempting to reference a deleted function
				Assert::IsTrue( *p11 == 6 );
				foo1(std::move(p11));//rvalue, nothing but
				Assert::IsTrue( *p11 == 6 );
				auto p24 = foo1(std::move(p11));//move it
				Assert::IsTrue( p11 == nullptr );
				Assert::IsTrue( *p24 == 6 );
			}
		}
	};
	TEST_CLASS(UT_ReadWriteData)
	{
	public:
		TEST_METHOD(UT_WriteDataReadData)
		{
			CMemFile file;
			auto source_interface	= ReadWrite_CFile(file);
			auto dest_interface		= ReadWrite_CFile(file);

			WriteData( dynamic_cast<CFile&>(file), 5i32 );
			WriteData( file, 6i16 );
			WriteData( dest_interface, 7i64 );

			WriteData( file, (char *)nullptr);
			WriteData( file, (char const*)"hallo");
			WriteData( file, (wchar_t const*)L"hallo");
			WriteData( file, "hallo");//schreibt das array mit 5 zeichen ohne 0-terminierung, wenn ReadWrite_char_t_array.h includiert

			WriteData( file, strlen("hallo"));
			_WriteData( file, "hallo", strlen("hallo") );//schreibt die zeichen bis zur 0-terminierung. 

			wchar_t chararray5[5]{L'h',L'a',L'l',L'l',L'o'};//nicht 0-terminiert
			WriteData( file, chararray5);//schreibt das array mit 5 zeichen
			wchar_t chararray20[20]{L"hallo"};// 0-terminiert
			WriteData( file, chararray20);//schreibt das array mit 5 zeichen


			int int_values[]{3,1,0,2};
			//auto ptr = int_values;
			//WriteData( file, ptr );//error C2607: static assertion failed. so soll es sein
			WriteData( file, int_values );

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			Assert::IsTrue( ReadData<__int32>( file ) == 5 );
			__int16 value;ReadData( source_interface, value );Assert::IsTrue( value==6 );
			Assert::IsTrue( ReadData<__int64>( dynamic_cast<CFile&>(file) ) == 7 );
			std::unique_ptr<char[]> text{ ReadData<char*>( file ) };
			Assert::IsTrue( text == nullptr );
			ReadData( file, text );
			Assert::IsTrue( strcmp(text.get(),"hallo")==0);
			std::unique_ptr<wchar_t[]> ltext{ ReadData<wchar_t*>( file ) };
			Assert::IsTrue( wcscmp(ltext.get(),L"hallo")==0);
			//ReadData<char[6]>( file );
			char h6[_countof("hallo")];
			ReadData( file, h6 );//liest das array mit 6-zeichen
			Assert::IsTrue( memcmp(h6,"hallo",sizeof(h6))==0);
			char h5[_countof("hallo")-1];
			ReadData( file, h5);//liest erst die geschriebene l�nge, dann die daten des arrays, 5-zeichen
			Assert::IsTrue( memcmp(h5,"hallo",sizeof(h5))==0);

			wchar_t chararray5_in[5];
			ReadData( file, chararray5_in );//liest das array mit 5-zeichen nicht null-terminiert
			Assert::IsTrue( stringncmp(chararray5_in,chararray5,_countof(chararray5))==0 );
			wchar_t chararray20_in[20];
			ReadData( file, chararray20_in );//liest das array mit 5-zeichen null-terminiert
			Assert::IsTrue( stringncmp(chararray20_in,chararray20,_countof(chararray20))==0 );
			


			int int_values2[]{0,1,2,3};
			ReadData( file, int_values2 );
			Assert::IsTrue( memcmp(int_values,int_values2,sizeof(int_values2))==0 );


			Assert::IsTrue( file.GetPosition() == written );
		}
		TEST_METHOD(UT_WriteDataReadData_exception)
		{
			CMemFile file;
			CFile* pFile = &file;

			auto value = 0x0102030405060708i64;
			WriteData( file, value );

			file.Seek(0,CFile::begin);

			ReadData( file, value );
			try
			{
				ReadData( pFile, value );
				Assert::Fail(L"exception erwartet");
			}
			catch(std::exception & e )
			{
				Logger::WriteMessage(typeid(e).name());
				auto p { e.what() };
				Logger::WriteMessage(p);
			}


		}
		TEST_METHOD(UT_WriteDataReadData_int64)
		{
			CMemFile file;
			CFile* pFile = &file;

				auto value = 0x0102030405060708i64;
			WriteData( file, value );

				auto written = file.GetPosition();
				Assert::IsTrue( written == sizeof(value) );

				file.Seek(0,CFile::begin);

			auto valueRead = ReadData<decltype(value)>(file);
				Assert::IsTrue( value == valueRead );

				Assert::IsTrue( file.GetPosition() == written );
				file.Seek(0,CFile::begin);

				valueRead=0;
			ReadData(pFile, valueRead);
				Assert::IsTrue( value == valueRead );

				Assert::IsTrue( file.GetPosition() == written );
				file.Seek(0,CFile::begin);
		}
		TEST_METHOD(UT_WriteDataReadData_WS_auto_ptr)
		{
			CMemFile file;
			CFile* pFile = &file;

				WS::auto_ptr<short> value;
			WriteData( file, value );
				value = std::make_unique<decltype(value)::element_type>( 5i16 );
			WriteData( file, value );

				auto written = file.GetPosition();
				file.Seek(0,CFile::begin);

			auto valueRead = ReadData<decltype(value)>(file);
				Assert::IsTrue( valueRead == nullptr);
			ReadData(pFile,valueRead);
				Assert::IsTrue( *valueRead == *value );

				Assert::IsTrue( file.GetPosition() == written );
				file.Seek(0,CFile::begin);
		}
		TEST_METHOD(UT_WriteDataReadData_std_unique_ptr)
		{
			CMemFile file;
			CFile* pFile = &file;

				std::unique_ptr<short> value;
			WriteData( file, value );
				value = std::make_unique<decltype(value)::element_type>( 5i16 );
			WriteData( file, value );

				auto written = file.GetPosition();
				file.Seek(0,CFile::begin);

			auto valueRead = ReadData<decltype(value)>(file);
				Assert::IsTrue( valueRead == nullptr);
			ReadData(pFile,valueRead);
				Assert::IsTrue( *valueRead == *value );

				Assert::IsTrue( file.GetPosition() == written );
				file.Seek(0,CFile::begin);
		}
		TEST_METHOD(UT_WriteDataReadData_std_shared_ptr)
		{
			CMemFile file;
			CFile* pFile = &file;

				std::shared_ptr<short> value;
			WriteData( file, value );
				value = std::make_unique<decltype(value)::element_type>( 5i16 );
			WriteData( file, value );

				auto written = file.GetPosition();
				file.Seek(0,CFile::begin);

			auto valueRead = ReadData<decltype(value)>(file);
				Assert::IsTrue( valueRead == nullptr);
			ReadData(pFile,valueRead);
				Assert::IsTrue( *valueRead == *value );

				Assert::IsTrue( file.GetPosition() == written );
				file.Seek(0,CFile::begin);
		}
		TEST_METHOD(UT_WriteDataReadData_signed_char_array)//signed char und unsigned char sind kein char_type also immer das ganze array speichern
		{
			CMemFile file;
			CFile* pFile = &file;

				signed char value[4]{2,3,0,1};
			WriteData( file, value );

				auto written = file.GetPosition();
				Assert::IsTrue( written == sizeof(value) );

				file.Seek(0,CFile::begin);

				decltype(value) valueRead;
			ReadData(pFile, valueRead);
				Assert::IsTrue( memcmp(value,valueRead,sizeof(valueRead))==0 );

				Assert::IsTrue( file.GetPosition() == written );
				file.Seek(0,CFile::begin);
		}
		TEST_METHOD(UT_WriteDataReadData_char_array)//char ist ein char_type, dann wird ggf nur bis zur 0-ternminierung gespeichert
		{
			CMemFile file;
			CFile* pFile = &file;

			char value[4]{2,3,0,1};
			WriteData( pFile, value );//bis zur 0

			auto written = file.GetPosition();

			file.Seek(0,CFile::begin);

			decltype(value) valueRead;
			ReadData(file, valueRead);
			Assert::IsTrue( stringncmp(value,valueRead,sizeof(valueRead))==0 );

			Assert::IsTrue( file.GetPosition() == written );
			file.Seek(0,CFile::begin);
		}
		TEST_METHOD(UT_WriteDataReadData_string)//char und wchar_t sind z.zt. strings
		{
			CMemFile file;
			CFile* pFile = &file;

			char const * value = nullptr;
			WriteData( pFile, value );//nullptr
			value = "";
			WriteData( pFile, value );//leerstring
			value = "hallo";
			WriteData( pFile, value );//bis zur 0

			auto written = file.GetPosition();

			file.Seek(0,CFile::begin);

			std::unique_ptr<std::remove_cv_t<std::remove_pointer_t<decltype(value)>>[]> valueRead ;//[] wichtig 
			ReadData(file, valueRead);
			Assert::IsTrue( valueRead==nullptr );
			ReadData(file, valueRead);
			Assert::IsTrue( stringcmp("",valueRead.get())==0 );
			ReadData(file, valueRead);
			Assert::IsTrue( stringcmp(value,valueRead.get())==0 );

			Assert::IsTrue( file.GetPosition() == written );
			file.Seek(0,CFile::begin);
		}
		TEST_METHOD(UT_WriteDataReadData_struct)
		{
			CMemFile file;
			CFile* pFile = &file;
			
			A value{
				5
				,6
				,std::unique_ptr<char[]>{_strdup("hallo")}
				,std::unique_ptr<wchar_t[]>{_wcsdup(L"welt")}
				,std::make_unique<unsigned short>(5ui16) };

			WriteData( pFile, value );//nullptr

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			A valueRead;

			ReadData(pFile, valueRead);
			Assert::IsTrue( valueRead==value );

			Assert::IsTrue( file.GetPosition() == written );
			file.Seek(0,CFile::begin);
		}
		TEST_METHOD(UT_WriteDataReadData_struct_stream)
		{
			char buf[1000]{};

			OMemStream out(buf,_countof(buf));
			IMemStream in(buf,_countof(buf));

			A value{
				5
				,6
				,std::unique_ptr<char[]>{_strdup("hallo")}
			,std::unique_ptr<wchar_t[]>{_wcsdup(L"welt")}
			,std::make_unique<unsigned short>(5ui16) };

			WriteData( Write_Stream(out), value );//nullptr

			A valueRead;

			ReadData(Read_Stream(in), valueRead);
			Assert::IsTrue( valueRead==value );
		}
		TEST_METHOD(UT_WriteDataReadData_struct_als_unique_ptr)
		{
			CMemFile file;
			CFile* pFile = &file;

			auto value = std::unique_ptr<A>{ new A
											 {
												5
												,6
												,std::unique_ptr<char[]>{_strdup("hallo")}
												,std::unique_ptr<wchar_t[]>{_wcsdup(L"welt")}
												,std::make_unique<unsigned short>(5ui16) 
											 }
										   };
			WriteData( ReadWrite_CFile(file), decltype(value){}  );//nullptr
			WriteData( pFile, value );
			WriteData( pFile, *value );
			WriteData( pFile, value );

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			static_assert( decltype(HasMethod_Load<A,CFile*>(0))::value );
			auto valueRead = ReadData<decltype(value)>(pFile);
			Assert::IsTrue( valueRead==nullptr );
			ReadData(ReadWrite_CFile(file), valueRead);
			Assert::IsTrue( dereferenced::equal(valueRead,value) );
			auto valueRead2  = ReadData<A>( &file );
			Assert::IsTrue( valueRead2 == *value );
			ReadData(&file, valueRead);
			Assert::IsTrue( dereferenced::equal(valueRead,value) );

			Assert::IsTrue( file.GetPosition() == written );
			file.Seek(0,CFile::begin);
		}
		TEST_METHOD(UT_WriteDataReadData_vector)
		{
			CMemFile file;
			using container_t = std::vector<int>;
			container_t container1;

			WriteData(file,container1);
			auto container2 = container_t{1,3,5,2,4,6};
			WriteData(file,container2);

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			auto read = ReadData<container_t>(file);
			Assert::IsTrue( read == container1 );
			ReadData(file,read);
			Assert::IsTrue( read == container2 );

			Assert::IsTrue( file.GetPosition() == written );
		}
		TEST_METHOD(UT_WriteDataReadData_deque)
		{
			CMemFile file;
			using container_t = std::deque<int>;
			container_t container1;

			WriteData(file,container1);
			auto container2 = container_t{1,3,5,2,4,6};
			WriteData(file,container2);

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			auto read = ReadData<container_t>(file);
			Assert::IsTrue( read == container1 );
			ReadData(file,read);
			Assert::IsTrue( read == container2 );

			Assert::IsTrue( file.GetPosition() == written );
		}
		TEST_METHOD(UT_WriteDataReadData_set)
		{
			CMemFile file;
			using container_t = std::set<int>;
			container_t container1;

			WriteData(file,container1);
			auto container2 = container_t{1,3,5,2,4,6};
			WriteData(file,container2);

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			auto read = ReadData<container_t>(file);
			Assert::IsTrue( read == container1 );
			ReadData(file,read);
			Assert::IsTrue( read == container2 );

			Assert::IsTrue( file.GetPosition() == written );
		}
		TEST_METHOD(UT_WriteDataReadData_map)
		{
			CMemFile file;
			using container_t = std::map<int,int>;
			container_t container1;

			WriteData(file,container1);
			auto container2 = container_t{{1,1},{2,3},{3,5},{4,2},{5,4},{6,6}};
			WriteData(file,container2);

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			auto read = ReadData<container_t>(file);
			Assert::IsTrue( read == container1 );
			ReadData(file,read);
			Assert::IsTrue( read == container2 );

			Assert::IsTrue( file.GetPosition() == written );
		}
		TEST_METHOD(UT_WriteDataReadData_unorderdmap)
		{
			CMemFile file;
			using container_t = std::unordered_map<int,int>;
			container_t container1;

			WriteData(file,container1);
			auto container2 = container_t{{1,1},{2,3},{3,5},{4,2},{5,4},{6,6}};
			WriteData(file,container2);

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			auto read = ReadData<container_t>(file);
			Assert::IsTrue( read == container1 );
			ReadData(file,read);
			Assert::IsTrue( read == container2 );

			Assert::IsTrue( file.GetPosition() == written );
		}
		TEST_METHOD(UT_WriteDataReadData_multimap)
		{
			CMemFile file;
			using container_t = std::multimap<int,int>;
			container_t container1;

			WriteData(file,container1);
			auto container2 = container_t{{1,1},{1,3},{1,5},{2,2},{2,4},{2,6}};
			WriteData(file,container2);

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			auto read = ReadData<container_t>(file);
			Assert::IsTrue( read == container1 );
			ReadData(file,read);
			Assert::IsTrue( read == container2 );

			Assert::IsTrue( file.GetPosition() == written );
		}
		TEST_METHOD(UT_WriteDataReadData_mit_load_ctor)
		{
			CMemFile file;

			load_ctor data{6};
			
			WriteData(&file,data);
			WriteData(&file,data);

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			auto read = ReadData<load_ctor>(&file);
			Assert::IsTrue( read.i == data.i );
			std::string str{"hallo"};
			read = ReadData<load_ctor>(&file,5,str);
			Assert::IsTrue( read.i == data.i );

			Assert::IsTrue( file.GetPosition() == written );
		}
		TEST_METHOD(UT_WriteDataReadData_mit_load_bool_ctor)
		{
			CMemFile file;

			load_bool_ctor data{short(6)};

			WriteData(&file,data);

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			auto read = ReadData<load_bool_ctor>(&file);
			Assert::IsTrue( read.i == data.i );

			Assert::IsTrue( file.GetPosition() == written );
		}
		TEST_METHOD(UT_WriteDataReadData_mit_load_ctor_with_params)
		{
			CMemFile file;

			load_with_params data{short(6)};
			load_with_params read;

			WriteData(&file,data);
			WriteData(&file,data);

			auto written = file.GetPosition();
			file.Seek(0,CFile::begin);

			auto ptr = std::make_unique<int>(4);

			ReadData(&file, read, std::move(ptr));
			Assert::IsTrue( read.i == data.i );
			Assert::IsTrue( read.ptr == ptr.get() );

			ReadData(&file, read, ptr);
			Assert::IsTrue( read.i == data.i );
			Assert::IsTrue( read.ptr == ptr.get() );

			Assert::IsTrue( file.GetPosition() == written );
		}
	};
}

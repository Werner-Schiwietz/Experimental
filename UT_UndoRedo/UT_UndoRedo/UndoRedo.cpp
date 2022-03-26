#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define USE_STD_STRING
#ifdef USE_STD_STRING
	#define UndoRedo_StringType std::string
	#define _Text(t) t
#else
	//UndoRedo_StringType not nessessary, default string_type is std::wstring
	//#define UndoRedo_StringType std::wstring
	#define _Text(t) L ## t
#endif


#include "UndoRedoAction.h"
#include "..\..\..\WernersTools\headeronly\char_helper.h"


//auto x = tostring<char const *>(5);//error C2338: it must be a string-class like std::wstring CString ...

namespace UT_UndoRedo
{
	TEST_CLASS(UndoRedoTest)
	{
	public:
		
		TEST_METHOD(UndoRedo_VWHoldAllRedos)
		{
			int i=0;
			auto undo_redoPtr = UndoRedo::CreateInterface<UndoRedo::VWHoldAllRedos>();

			struct doing
			{
				int & i;
				int newvalue{};
				doing(int & i, int newvalue) : i(i), newvalue(newvalue){}
				void operator()(){this->i = this->newvalue;}
			};

			int newvalue = 2;
			(*undo_redoPtr).AddAndDo(doing{i,newvalue},[&i,oldvalue=i]()->void {i=oldvalue;},std::make_shared<UndoRedo::DoingText>(tostring<UndoRedo::string_t>(i),tostring<UndoRedo::string_t>(newvalue)));
			Assert::IsTrue(i==newvalue);
			newvalue = 4;
			(*undo_redoPtr).AddAndDo(doing{i,newvalue},[&i,oldvalue=i]()->void {i=oldvalue;},std::make_shared<UndoRedo::DoingText>(tostring<UndoRedo::string_t>(i),tostring<UndoRedo::string_t>(newvalue)));
			Assert::IsTrue(i==newvalue);
			newvalue = 10;
			(*undo_redoPtr).AddAndDo(doing{i,newvalue},[&i,oldvalue=i]()->void {i=oldvalue;},std::make_shared<UndoRedo::DoingText>(tostring<UndoRedo::string_t>(i),tostring<UndoRedo::string_t>(newvalue)));
			Assert::IsTrue(i==newvalue);
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==4);
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==2);


			Assert::IsTrue(i==2);
			newvalue = 1;
			(*undo_redoPtr).AddAndDo(doing{i,newvalue},[&i,oldvalue=i]()->void {i=oldvalue;},std::make_shared<UndoRedo::DoingText>(tostring<UndoRedo::string_t>(i),tostring<UndoRedo::string_t>(newvalue)));//die jetzigen redos  werden doppelt auf den undo-stack verschoben. damit bleibt jegliche jeweils gemachte aktion erhalten
			Assert::IsTrue(i==newvalue);

			auto i_equ_FirstUndo = [&]()
			{
				auto t_undo = (*undo_redoPtr).UndoTexte();
				[[maybe_unused]]
				auto t_redo = (*undo_redoPtr).RedoTexte();

				if( ((*undo_redoPtr).Undo()) == false )
					return false;

				Assert::IsTrue(i==stringtoi(t_undo[0].c_str() ));
				return true;
			};
			while(i_equ_FirstUndo()){}
		}
		TEST_METHOD(UndoRedo_IntIncDec)
		{
			int i=0;
			auto undo_redoPtr = UndoRedo::CreateInterface();

			auto undo = [&i]()->void 
			{
				--i;
			};
			auto doing = [&i]()->void 
			{
				++i;
			};

			Assert::IsTrue(i==0);
			(*undo_redoPtr).AddAndDo(doing,undo);
			Assert::IsTrue(i==1);
			(*undo_redoPtr).AddAndDo(doing,undo);
			Assert::IsTrue(i==2);
			(*undo_redoPtr).AddAndDo(doing,undo);
			Assert::IsTrue(i==3);

			Assert::IsFalse((*undo_redoPtr).Redo());

			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue((*undo_redoPtr).Redo());
			Assert::IsTrue(i==3);

			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==1);
			Assert::IsTrue((*undo_redoPtr).Redo());
			Assert::IsTrue(i==2);
			Assert::IsTrue((*undo_redoPtr).Redo());
			Assert::IsTrue(i==3);

			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==1);
			(*undo_redoPtr).AddAndDo(doing,undo);
			Assert::IsTrue(i==2);
			Assert::IsFalse((*undo_redoPtr).Redo());
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==1);
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==0);
			Assert::IsFalse((*undo_redoPtr).Undo());
		}
		TEST_METHOD(TestMethod_Texte)
		{
			int i=0;
			auto undo_redoPtr = UndoRedo::CreateInterface();

			auto undo = [&i]()->void 
			{
				--i;
			};
			auto doing = [&i]()->void 
			{
				++i;
			};

			(*undo_redoPtr).AddAndDo(doing,undo,std::make_shared<UndoRedo::DoingTextSimple>(tostring<UndoRedo::string_t>(i)));
			(*undo_redoPtr).AddAndDo(doing,undo,std::make_shared<UndoRedo::DoingTextSimple>(tostring<UndoRedo::string_t>(i)));
			(*undo_redoPtr).AddAndDo(doing,undo,std::make_shared<UndoRedo::DoingTextSimple>(tostring<UndoRedo::string_t>(i)));

			auto texte = (*undo_redoPtr).UndoTexte();
			Assert::IsTrue(texte.size()==3);
			texte = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue(texte.size()==0);

			(*undo_redoPtr).Undo();

			texte = (*undo_redoPtr).UndoTexte();
			Assert::IsTrue(texte.size()==2);
			texte = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue(texte.size()==1);

			(*undo_redoPtr).Redo();

			texte = (*undo_redoPtr).UndoTexte();
			Assert::IsTrue(texte.size()==3);
			texte = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue(texte.size()==0);

			(*undo_redoPtr).Undo();

			texte = (*undo_redoPtr).UndoTexte();
			Assert::IsTrue(texte.size()==2);
			texte = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue(texte.size()==1);

			(*undo_redoPtr).Undo();

			texte = (*undo_redoPtr).UndoTexte();
			Assert::IsTrue(texte.size()==1);
			texte = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue(texte.size()==2);

			(*undo_redoPtr).Undo();

			texte = (*undo_redoPtr).UndoTexte();
			Assert::IsTrue(texte.size()==0);
			texte = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue(texte.size()==3);

			(*undo_redoPtr).AddAndDo(doing,undo);
			texte = (*undo_redoPtr).UndoTexte();
			Assert::IsTrue( stringcmp(texte[0].c_str(),_Text("Undo"))==0);
			(*undo_redoPtr).Undo();
			texte = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue( stringcmp(texte[0].c_str(),_Text("Redo"))==0);
		}
	};
}

#include <atlstr.h>
namespace UT_tostring
{
	TEST_CLASS(tostringTest)
	{
		TEST_METHOD(UT_tostring)
		{
			{	//_mypow test wird aber nicht weiter verwendet
				[[maybe_unused]] constexpr auto x =_mypow<2,8>();
				[[maybe_unused]] constexpr auto y =_mypow<x,sizeof(_int16)>();
				[[maybe_unused]] constexpr auto z =_mypow<x,sizeof(_int32)>();
				[[maybe_unused]] constexpr auto z2 =_mypow<x,sizeof(_int32),_int64>();
				[[maybe_unused]] unsigned int e = unsigned int{256};
				[[maybe_unused]] unsigned int e2 = e*e;
				[[maybe_unused]] unsigned int e3 = e*e2;
				[[maybe_unused]] unsigned int e4 = e*e3;
			}

			Assert::IsTrue( tostring<std::string>(0765) == "501" ); //dezimal
			Assert::IsTrue( tostring<std::string,8>(0765) == "765" ); //octal
			Assert::IsTrue( tostring<std::string,16>(0765) == "1f5" ); //hex

			Assert::IsTrue( tostring<std::string>(_int32{11}) == "11" ); //standard-basis = 10
			Assert::IsTrue( tostring<std::string,2>(_int32{11}) == "1011" ); 
			Assert::IsTrue( tostring<std::string,16>(_int32{11}) == "b" ); 

			Assert::IsTrue( tostring<std::wstring,2>(_int64{0b1001'1001'0000'0000'1001'0000'1111'0101'1010}) ==L"100110010000000010010000111101011010" ); //binär
			Assert::IsTrue( tostring<std::string,16>(_int64{0b1001'1001'0000'0000'1001'0000'1111'0101'1010}) == "990090f5a" ); //hex
			Assert::IsTrue( tostring<std::string,8>(_int64{0b1001'1001'0000'0000'1001'0000'1111'0101'1010}) == "462002207532" ); //octal

			Assert::IsTrue( tostring<CStringA>(_int32{11}) == "11" ); //standard-basis = 10
			Assert::IsTrue( tostring<CStringA,2>(_int32{11}) == "1011" ); 
			Assert::IsTrue( tostring<CStringA,16>(_int32{11}) == "b" ); 

			Assert::IsTrue( tostring<CStringW,2>(_int64{0b1001'1001'0000'0000'1001'0000'1111'0101'1010}) ==L"100110010000000010010000111101011010" ); //binär
			Assert::IsTrue( tostring<CStringA,16>(_int64{0b1001'1001'0000'0000'1001'0000'1111'0101'1010}) == "990090f5a" ); //hex
			Assert::IsTrue( tostring<CStringA,8>(_int64{0b1001'1001'0000'0000'1001'0000'1111'0101'1010}) == "462002207532" ); //octal

		}
	};
}



#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "UndoRedoAction.h"

namespace
{
	int stringtoi( char const * psz )
	{
		return atoi( psz );
	}
	int stringtoi( wchar_t const * psz )
	{
		return _wtoi( psz );
	}
}

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

			(*undo_redoPtr).AddAndDo(doing{i,2},[&i,oldvalue=i]()->void {i=oldvalue;},std::make_shared<UndoRedo::DoingText>(L"0",L"2"));
			Assert::IsTrue(i==2);
			(*undo_redoPtr).AddAndDo(doing{i,4},[&i,oldvalue=i]()->void {i=oldvalue;},std::make_shared<UndoRedo::DoingText>(L"2",L"4"));
			Assert::IsTrue(i==4);
			(*undo_redoPtr).AddAndDo(doing{i,10},[&i,oldvalue=i]()->void {i=oldvalue;},std::make_shared<UndoRedo::DoingText>(L"4",L"10"));
			Assert::IsTrue(i==10);
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==4);
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==2);
			auto t_undo = (*undo_redoPtr).UndoTexte();
			auto t_redo = (*undo_redoPtr).RedoTexte();
			auto i_equ_FirstUndo = [&]()
			{
				return i==stringtoi(t_undo[0].c_str() );
			};

			Assert::IsTrue(i==2);
			(*undo_redoPtr).AddAndDo(doing{i,1},[&i,oldvalue=i]()->void {i=oldvalue;},std::make_shared<UndoRedo::DoingText>(L"2",L"1"));//die jetzigen redos  werden doppelt auf den undo-stack verschoben. damit bleibt jegliche jeweils gemachte aktion erhalten
			Assert::IsTrue(i==1);
			t_undo = (*undo_redoPtr).UndoTexte();
			t_redo = (*undo_redoPtr).RedoTexte();

			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(i_equ_FirstUndo());

			t_undo = (*undo_redoPtr).UndoTexte();
			t_redo = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==4);
			Assert::IsTrue(i_equ_FirstUndo());

			t_undo = (*undo_redoPtr).UndoTexte();
			t_redo = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==10);
			Assert::IsTrue(i_equ_FirstUndo());

			t_undo = (*undo_redoPtr).UndoTexte();
			t_redo = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==4);
			Assert::IsTrue(i_equ_FirstUndo());

			t_undo = (*undo_redoPtr).UndoTexte();
			t_redo = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(i_equ_FirstUndo());

			t_undo = (*undo_redoPtr).UndoTexte();
			t_redo = (*undo_redoPtr).RedoTexte();
			Assert::IsTrue((*undo_redoPtr).Undo());
			Assert::IsTrue(i==0);
			Assert::IsTrue(i_equ_FirstUndo());

			t_undo = (*undo_redoPtr).UndoTexte();
			t_redo = (*undo_redoPtr).RedoTexte();
			Assert::IsFalse((*undo_redoPtr).Undo());
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

			(*undo_redoPtr).AddAndDo(doing,undo,std::make_shared<UndoRedo::DoingTextSimple>(L"0"));
			(*undo_redoPtr).AddAndDo(doing,undo,std::make_shared<UndoRedo::DoingTextSimple>(L"1"));
			(*undo_redoPtr).AddAndDo(doing,undo,std::make_shared<UndoRedo::DoingTextSimple>(L"2"));

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
		}
	};
}


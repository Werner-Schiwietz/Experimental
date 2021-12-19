#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "UndoRedoAction.h"

namespace UT_UndoRedo
{
	TEST_CLASS(UndoRedoTest)
	{
	public:
		
		TEST_METHOD(UndoRedo_IntIncDec)
		{
			int i=0;
			UndoRedo::VW undo_redo;

			auto undo = [&i]()->void 
			{
				--i;
			};
			auto doing = [&i]()->void 
			{
				++i;
			};

			Assert::IsTrue(i==0);
			undo_redo.AddAndDo(doing,undo);
			Assert::IsTrue(i==1);
			undo_redo.AddAndDo(doing,undo);
			Assert::IsTrue(i==2);
			undo_redo.AddAndDo(doing,undo);
			Assert::IsTrue(i==3);

			Assert::IsFalse(undo_redo.Redo());

			Assert::IsTrue(undo_redo.Undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(undo_redo.Redo());
			Assert::IsTrue(i==3);

			Assert::IsTrue(undo_redo.Undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(undo_redo.Undo());
			Assert::IsTrue(i==1);
			Assert::IsTrue(undo_redo.Redo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(undo_redo.Redo());
			Assert::IsTrue(i==3);

			Assert::IsTrue(undo_redo.Undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(undo_redo.Undo());
			Assert::IsTrue(i==1);
			undo_redo.AddAndDo(doing,undo);
			Assert::IsTrue(i==2);
			Assert::IsFalse(undo_redo.Redo());
			Assert::IsTrue(undo_redo.Undo());
			Assert::IsTrue(i==1);
			Assert::IsTrue(undo_redo.Undo());
			Assert::IsTrue(i==0);
			Assert::IsFalse(undo_redo.Undo());
		}
		TEST_METHOD(TestMethod_Texte)
		{
			int i=0;
			UndoRedo::VW undo_redo;

			auto undo = [&i]()->void 
			{
				--i;
			};
			auto doing = [&i]()->void 
			{
				++i;
			};

			undo_redo.AddAndDo(doing,undo,std::make_shared<UndoRedo::DoingTextSimple>(L"0"));
			undo_redo.AddAndDo(doing,undo,std::make_shared<UndoRedo::DoingTextSimple>(L"1"));
			undo_redo.AddAndDo(doing,undo,std::make_shared<UndoRedo::DoingTextSimple>(L"2"));

			auto texte = undo_redo.UndoTexte();
			Assert::IsTrue(texte.size()==3);
			texte = undo_redo.RedoTexte();
			Assert::IsTrue(texte.size()==0);

			undo_redo.Undo();

			texte = undo_redo.UndoTexte();
			Assert::IsTrue(texte.size()==2);
			texte = undo_redo.RedoTexte();
			Assert::IsTrue(texte.size()==1);

			undo_redo.Redo();

			texte = undo_redo.UndoTexte();
			Assert::IsTrue(texte.size()==3);
			texte = undo_redo.RedoTexte();
			Assert::IsTrue(texte.size()==0);

			undo_redo.Undo();

			texte = undo_redo.UndoTexte();
			Assert::IsTrue(texte.size()==2);
			texte = undo_redo.RedoTexte();
			Assert::IsTrue(texte.size()==1);

			undo_redo.Undo();

			texte = undo_redo.UndoTexte();
			Assert::IsTrue(texte.size()==1);
			texte = undo_redo.RedoTexte();
			Assert::IsTrue(texte.size()==2);

			undo_redo.Undo();

			texte = undo_redo.UndoTexte();
			Assert::IsTrue(texte.size()==0);
			texte = undo_redo.RedoTexte();
			Assert::IsTrue(texte.size()==3);
		}
	};
}


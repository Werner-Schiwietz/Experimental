#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "UndoRedoAction.h"

namespace UT_UndoRedo
{
	TEST_CLASS(UndoRedoTest)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			int i=0;
			UndoRedo::VW undo_redo;

			auto undo = [&i]()->void 
			{
				--i;
			};
			auto redo = [&i]()->void 
			{
				++i;
			};

			auto _do = [&]()
			{
				redo();//action ausführen
				undo_redo.add(undo,redo);//
			};

			Assert::IsTrue(i==0);
			_do();
			Assert::IsTrue(i==1);
			_do();
			Assert::IsTrue(i==2);
			_do();
			Assert::IsTrue(i==3);

			Assert::IsFalse(undo_redo.redo());

			Assert::IsTrue(undo_redo.undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(undo_redo.redo());
			Assert::IsTrue(i==3);

			Assert::IsTrue(undo_redo.undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(undo_redo.undo());
			Assert::IsTrue(i==1);
			Assert::IsTrue(undo_redo.redo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(undo_redo.redo());
			Assert::IsTrue(i==3);

			Assert::IsTrue(undo_redo.undo());
			Assert::IsTrue(i==2);
			Assert::IsTrue(undo_redo.undo());
			Assert::IsTrue(i==1);
			_do();
			Assert::IsTrue(i==2);
			Assert::IsFalse(undo_redo.redo());
			Assert::IsTrue(undo_redo.undo());
			Assert::IsTrue(i==1);
			Assert::IsTrue(undo_redo.undo());
			Assert::IsTrue(i==0);
			Assert::IsFalse(undo_redo.undo());
		}
	};
}

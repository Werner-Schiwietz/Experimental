#pragma once

#include <functional>
#include <stack>
#include <string>
#include <stdexcept>
#include <memory>

namespace UndoRedo
{
	enum Direction
	{
		Undo,
		Redo
	};
	struct IDoingText
	{
		using string_t = std::wstring;
		virtual ~IDoingText(){}

		virtual string_t operator()(Direction) = 0;
	};
	class DoingTextNone : public IDoingText
	{
	public:
		virtual string_t operator()(Direction direction) override 
		{
			switch(direction)
			{
			case Direction::Undo:
				return L"Undo";
			case Direction::Redo:
				return L"Redo";
			}
			throw std::invalid_argument( __FUNCSIG__ " direction unknown " );
		}
	};
	class DoingTextSimple : public IDoingText
	{
		string_t text;
	public:
		DoingTextSimple(string_t text) : text(std::move(text)){}
		virtual string_t operator()(Direction) override { return text; }
	};
	class DoingText : public IDoingText
	{
		string_t textUndo;
		string_t textRedo;
	public:
		DoingText(string_t textUndo, string_t textRedo) : textUndo(std::move(textUndo)),textRedo(std::move(textRedo)){}
		virtual string_t operator()(Direction direction) override 
		{ 
			switch(direction)
			{
			case Direction::Undo:
				return textUndo;
			case Direction::Redo:
				return textRedo;
			}
			throw std::invalid_argument( __FUNCSIG__ " direction unknown " );
		}
	};

	class UndoRedoAction
	{
	public:
		using action_t = std::function<void( void )>;
		using string_t = IDoingText::string_t;
	private:
		Direction					direction {Direction::Undo};
		action_t					undo_action;
		action_t					redo_action;
		std::shared_ptr<IDoingText>	doingtextPtr;

	public:
		UndoRedoAction( action_t undo_action, action_t redo_action, std::shared_ptr<IDoingText> doingtextPtr ) : undo_action( std::move( undo_action ) ), redo_action( std::move( redo_action ) ), doingtextPtr( std::move( doingtextPtr ) ) {}
		UndoRedoAction( action_t undo_action, action_t redo_action ) : UndoRedoAction( std::move(undo_action), std::move(redo_action), std::make_shared<DoingTextNone>() ){}

		void operator()();
		string_t const & doing_text();
		UndoRedoAction toggle() && ;
	};

	class VW
	{
	public:
		using action_t = UndoRedoAction::action_t;
	protected:
		std::stack<UndoRedoAction> undos;
		std::stack<UndoRedoAction> redos;

	public:
		void add( action_t undo, action_t redo );
		void add( action_t undo, action_t redo, std::shared_ptr<IDoingText>);

		bool undo();
		bool redo();

	protected:
		static bool		_action( std::stack<UndoRedoAction> & from, std::stack<UndoRedoAction> & to );
		virtual void	_handle_redos();
	};

	inline void UndoRedoAction::operator()()
	{
		if( this->direction == Direction::Redo )
			redo_action();
		else
			undo_action();
	}
	inline UndoRedoAction UndoRedoAction::toggle() && 
	{
		if( this->direction == Direction::Redo )
			this->direction = Direction::Undo;
		else
			this->direction = Direction::Redo;
		return std::move(*this);
	}

	inline void VW::_handle_redos()
	{
		//simple, clear redos
		decltype(this->redos){}.swap(this->redos);
	}
	inline void VW::add( action_t undo, action_t redo, std::shared_ptr<IDoingText> doingtext )
	{
		void handle_redos();
		undos.emplace( undo, redo, doingtext );
	}
	inline void VW::add( action_t undo, action_t redo )
	{
		_handle_redos();
		undos.emplace( undo, redo );
	}
	inline bool VW::_action( std::stack<UndoRedoAction> & from, std::stack<UndoRedoAction> & to )
	{
		if(from.size())
		{
			auto action = std::move(from.top());
			from.pop();
			action();
			to.emplace(std::move(action).toggle());
			return true;
		}
		return false;
	}
	inline bool VW::undo()
	{
		return _action( this->undos, this->redos );
	}
	inline bool VW::redo()
	{
		return _action( this->redos, this->undos );
	}
}

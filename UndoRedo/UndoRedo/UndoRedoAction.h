#pragma once

#include <functional>
#include <stack>
#include <string>
#include <stdexcept>
#include <memory>
#include <deque>

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

		virtual string_t const & operator()(Direction) const = 0;
	};
	class DoingTextNone : public IDoingText
	{
	public:
		virtual string_t const & operator()(Direction direction) const override 
		{
			static string_t const textUndo{L"Undo"};
			static string_t const textRedo{L"Redo"};

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
	class DoingTextSimple : public IDoingText
	{
		string_t text;
	public:
		DoingTextSimple(string_t text) : text(std::move(text)){}
		virtual string_t const & operator()(Direction) const override { return this->text; }
	};
	class DoingText : public IDoingText
	{
		string_t textUndo;
		string_t textRedo;
	public:
		DoingText(string_t textUndo, string_t textRedo) : textUndo(std::move(textUndo)),textRedo(std::move(textRedo)){}
		virtual string_t const & operator()(Direction direction) const override 
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

	class VW
	{
	protected:
		class Action
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
			Action( action_t undo_action, action_t redo_action, std::shared_ptr<IDoingText> doingtextPtr ) : undo_action( std::move( undo_action ) ), redo_action( std::move( redo_action ) ), doingtextPtr( std::move( doingtextPtr ) ) {}
			Action( action_t undo_action, action_t redo_action ) : Action( std::move(undo_action), std::move(redo_action), std::make_shared<DoingTextNone>() ){}

		public:
			Action				InvokeAndToggle() &&;
			string_t const &	DoingText(Direction) const;
		};
		class Stack : public std::stack<Action>
		{
		public:
			using base_t = std::stack<Action>;
			base_t::container_type const & GetContainer() const { return this->c; }
		};
		Stack undos;
		Stack redos;
	public:
		using action_t = Action::action_t;//void(void) no return-value, throw excetion 
		using textcontainer_t = std::deque<Action::string_t>;

	#pragma region public interface
		#pragma region adding new action
			void Add( action_t action, action_t undo );
			void Add( action_t action, action_t undo, std::shared_ptr<IDoingText>);
			void AddAndDo( action_t action, action_t undo );
			void AddAndDo( action_t action, action_t undo, std::shared_ptr<IDoingText>);
		#pragma endregion 

		#pragma region Undo-Redo-action
			//return-value false -> no action avaiable
			bool Undo();
			bool Redo();
		#pragma endregion 

		#pragma region UserInterface strings
			textcontainer_t UndoTexte() const {return _Texte(undos, Direction::Undo);}
			textcontainer_t RedoTexte() const {return _Texte(redos, Direction::Redo);}
		#pragma endregion 
	#pragma endregion 
	protected:
		static textcontainer_t	_Texte( Stack const &, Direction );
		static bool				_action( Stack & from, Stack & to );
		virtual void			_handle_redos();
	};

	inline VW::Action					VW::Action::InvokeAndToggle() &&	
	{
		if( this->direction == Direction::Redo )
		{
			redo_action();
			this->direction = Direction::Undo;
		}
		else
		{
			undo_action();
			this->direction = Direction::Redo;
		}
		return std::move( *this );
	}
	inline VW::Action::string_t const &	VW::Action::DoingText(Direction direction) const
	{
		return (*doingtextPtr)(direction);
	}

	inline void VW::_handle_redos()
	{
		//simple, clear redos
		decltype(this->redos){}.swap(this->redos);
	}
	inline bool VW::_action( Stack & from, Stack & to )
	{
		if(from.size())
		{
			auto action = std::move(from.top());
			from.pop();
			to.emplace(std::move(action).InvokeAndToggle());
			return true;
		}
		return false;
	}

	inline bool VW::Undo()
	{
		return _action( this->undos, this->redos );
	}
	inline bool VW::Redo()
	{
		return _action( this->redos, this->undos );
	}

	inline void VW::Add( action_t action, action_t undo, std::shared_ptr<IDoingText> doingtext )
	{
		_handle_redos();
		undos.emplace( undo, action, doingtext );
	}
	inline void VW::Add( action_t action, action_t undo)
	{
		_handle_redos();
		undos.emplace( undo, action );
	}
	inline void VW::AddAndDo( action_t action, action_t undo, std::shared_ptr<IDoingText> doingtext )
	{
		action();
		Add( action, undo, doingtext );
	}
	inline void VW::AddAndDo( action_t action, action_t undo)
	{
		action();
		Add( action, undo );
	}

	inline VW::textcontainer_t VW::_Texte( Stack const & data, Direction direction )
	{
		VW::textcontainer_t ret_value;
		for( auto iter = data.GetContainer().rbegin(); iter != data.GetContainer().rend(); ++iter )
		{
			ret_value.push_back( iter->DoingText(direction) );
		}
		return ret_value;
	}
}

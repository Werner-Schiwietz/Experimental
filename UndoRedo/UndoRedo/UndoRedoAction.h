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
	inline Direction Toggle( Direction direction )
	{
		if( direction == Direction::Redo )
			return Direction::Undo;
		else
			return Direction::Redo;
	}

	struct IDoingText
	{
		using string_t = std::wstring;
		virtual ~IDoingText(){}

		virtual string_t const &			operator()(Direction) const = 0;
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
	#pragma region internal classes and member
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
			void				Invoke();
			Action				MoveFromRedoToUndo() const &;//used by VWHoldAllRedos
			Action				Toggle() &&;//transfer undo <-> redo stack
			string_t const &	DoingText() const;
		};
		class Stack : public std::stack<Action>//gives VW access to std::stack-container
		{
		public:
			using base_t = std::stack<Action>;
			base_t::container_type const & GetContainer() const { return this->c; }
			void clear()
			{
				//moving data per swap to an temporary object
				std::remove_reference_t<decltype(*this)>{}.swap(*this);
			}
		};
		Stack undos;
		Stack redos;
	#pragma endregion

	#pragma region internal used datatypes
	public:
		using action_t = Action::action_t;//void(void) no return-value, throw exception 
		using textcontainer_t = std::deque<Action::string_t>;
	#pragma endregion

	#pragma region public interface
	public:
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
			textcontainer_t UndoTexte() const {return _Texte(undos);}
			textcontainer_t RedoTexte() const {return _Texte(redos);}
		#pragma endregion 
	#pragma endregion 

	#pragma region internal used methods
	protected:
		static textcontainer_t	_Texte( Stack const & );
		static bool				_action( Stack & from, Stack & to );
		virtual void			_handle_redos();//was passiert mit den redos bei add. default clear()
	#pragma endregion 
	};
	class VWHoldAllRedos : public VW
	{
	private:
		virtual void			_handle_redos() override;//umkopieren von Redo-Stack auf den Undo-Stack, statt clear()
	};

	inline void							VW::Action::Invoke() 
	{
		if( this->direction == Direction::Redo )
			redo_action();
		else
			undo_action();
	}
	inline VW::Action					VW::Action::Toggle() &&	
	{
		this->direction = UndoRedo::Toggle(this->direction);

		return std::move( *this );
	}
	inline VW::Action					VW::Action::MoveFromRedoToUndo() const &	
	{
		return *this;//just copy, used by VWHoldAllRedos::_handle_redos
	}
	inline VW::Action::string_t const &	VW::Action::DoingText() const
	{
		return (*doingtextPtr)(this->direction);
	}

	inline void VWHoldAllRedos::_handle_redos()
	{	//kein redo geht verloren
		std::remove_const_t<std::remove_reference_t<decltype(this->redos.GetContainer())>> data;

		while( this->redos.size() )
		{
			auto action = std::move(this->redos.top());//
			this->redos.pop();
			data.emplace_front(action.MoveFromRedoToUndo());
			this->undos.emplace(std::move(action).Toggle());
		}
		for( auto & action : data )
		{
			this->undos.emplace(std::move(action));
		}
	}
	inline void VW::_handle_redos()
	{
		//simple, clear redos, actions lost
		this->redos.clear();
	}

	inline bool VW::_action( Stack & from, Stack & to )
	{
		if(from.size())
		{
			auto action = std::move(from.top());//
			action.Invoke();
			from.pop();
			to.emplace(std::move(action).Toggle());
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

	 
	inline VW::textcontainer_t VW::_Texte( Stack const & data )//static
	{
		VW::textcontainer_t ret_value;
		for( auto iter = data.GetContainer().rbegin(); iter != data.GetContainer().rend(); ++iter )
		{
			ret_value.push_back( iter->DoingText() );
		}
		return ret_value;
	}
}

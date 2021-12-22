#pragma once

#include <functional>
#include <memory>
#include <deque>
#include <string>

#pragma region public interface
namespace UndoRedo
{
	enum class Direction;

#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ = 0
	struct IDoingText
	{
		using string_t = std::wstring;
		virtual ~IDoingText(){}

		virtual string_t const &			operator()(Direction) const _INTERFACE_FUNCTION_;
	};
#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ override


#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ = 0
	struct IPublic
	{
		using action_t = std::function<void( void )>;
		using string_t = IDoingText::string_t;
		using textcontainer_t = std::deque<IPublic::string_t>;
		virtual ~IPublic(){}

		#pragma region adding new action
			virtual void Add( action_t action, action_t undo ) _INTERFACE_FUNCTION_;
			virtual void Add( action_t action, action_t undo, std::shared_ptr<IDoingText>) _INTERFACE_FUNCTION_;
			virtual void AddAndDo( action_t action, action_t undo ) _INTERFACE_FUNCTION_;
			virtual void AddAndDo( action_t action, action_t undo, std::shared_ptr<IDoingText>) _INTERFACE_FUNCTION_;
		#pragma endregion 

		#pragma region Undo-Redo-action
			//return-value false -> no action avaiable
			virtual bool Undo() _INTERFACE_FUNCTION_;
			virtual bool Redo() _INTERFACE_FUNCTION_;
		#pragma endregion 

		#pragma region UserInterface strings 
			//next action at index 0
			virtual textcontainer_t UndoTexte() const  _INTERFACE_FUNCTION_;
			virtual textcontainer_t RedoTexte() const  _INTERFACE_FUNCTION_;
		#pragma endregion 

	};
#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ override

	class VW;//normal
	class VWHoldAllRedos;//extented losing no action
	std::unique_ptr<UndoRedo::IPublic> CreateInterface();//benutzt VW, also same as UndoRedo::CreateInterface<VW>()
	template<typename IPublic_Impl_t> std::unique_ptr<UndoRedo::IPublic> CreateInterface();//usage auto IPublicPtr = UndoRedo::CreateInterface<VWHoldAllRedos>();
}
#pragma endregion


#pragma region definition of public interface
#include <stack>
#include <stdexcept>

namespace UndoRedo//decalration
{
	enum class Direction
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

	class DoingTextNone : public IDoingText
	{
	public:
		virtual string_t const & operator()(Direction direction) const _INTERFACE_FUNCTION_
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
		virtual string_t const & operator()(Direction) const _INTERFACE_FUNCTION_ { return this->text; }
	};
	class DoingText : public IDoingText
	{
		string_t textUndo;
		string_t textRedo;
	public:
		DoingText(string_t textUndo, string_t textRedo) : textUndo(std::move(textUndo)),textRedo(std::move(textRedo)){}
		virtual string_t const & operator()(Direction direction) const _INTERFACE_FUNCTION_ 
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

	class VW : public IPublic
	{
	#pragma region internal classes and member
	protected:
		class Action
		{
		public:
			using action_t = IPublic::action_t;
			using string_t = IPublic::string_t;
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
		using action_t = IPublic::action_t;//void(void) no return-value, throw exception 
		using textcontainer_t = IPublic::textcontainer_t;
	#pragma endregion

	#pragma region public interface
	public:
		#pragma region adding new action
			void Add( action_t action, action_t undo ) _INTERFACE_FUNCTION_;
			void Add( action_t action, action_t undo, std::shared_ptr<IDoingText>) _INTERFACE_FUNCTION_;
			void AddAndDo( action_t action, action_t undo ) _INTERFACE_FUNCTION_;
			void AddAndDo( action_t action, action_t undo, std::shared_ptr<IDoingText>) _INTERFACE_FUNCTION_;
		#pragma endregion 

		#pragma region Undo-Redo-action
			//return-value false -> no action avaiable
			bool Undo() _INTERFACE_FUNCTION_;
			bool Redo() _INTERFACE_FUNCTION_;
		#pragma endregion 

		#pragma region UserInterface strings
			textcontainer_t UndoTexte() const  _INTERFACE_FUNCTION_ {return _Texte(undos);}
			textcontainer_t RedoTexte() const  _INTERFACE_FUNCTION_ {return _Texte(redos);}
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
}

namespace WS
{
	//reverse range-based for
	template<typename iterable_t> struct reverse
	{
		iterable_t & iterable;
		explicit reverse(iterable_t & iterable) : iterable(iterable){}

		auto begin() const noexcept(noexcept(std::rbegin(std::declval<iterable_t>()))){ return std::rbegin(this->iterable); }
		auto end() const noexcept(noexcept(std::rend(std::declval<iterable_t>()))){ return std::rend(this->iterable); }
	};
	template<typename container_t> auto Reverse(container_t & container){ return reverse<container_t>(container);}//komfortFuktion mit c++17 nicht mehr nötig
}

namespace UndoRedo//definition
{
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
	{	//move redos to undo-stack
		std::remove_const_t<std::remove_reference_t<decltype(this->redos.GetContainer())>> data;

		while( this->redos.size() )
		{
			auto action = std::move(this->redos.top());//
			this->redos.pop();
			data.emplace_front(action.MoveFromRedoToUndo());//realize undoing
			this->undos.emplace(std::move(action).Toggle());//move as undoing to undo-stack without action
		}
		for( auto & action : data )//move redoing to undo-stack without action
		{
			this->undos.emplace(std::move(action));
		}
	}
	inline void VW::_handle_redos()
	{
		//simple, clear redos, actions lost
		this->redos.clear();
	}

	inline bool VW::_action( Stack & from, Stack & to )//static
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
		for( decltype(auto) action : WS::Reverse(data.GetContainer()) )
		{
			ret_value.push_back( action.DoingText() );
		}
		//for( auto iter = data.GetContainer().rbegin(); iter != data.GetContainer().rend(); ++iter )
		//{
		//	ret_value.push_back( (*iter).DoingText() );
		//}
		return ret_value;
	}

	template<typename IPublic_Impl_t> std::unique_ptr<UndoRedo::IPublic> CreateInterface()
	{
		return std::make_unique<IPublic_Impl_t>();
	}
	template<> inline std::unique_ptr<UndoRedo::IPublic> CreateInterface<VWHoldAllRedos>()
	{
		return std::make_unique<VWHoldAllRedos>();
	}
	inline std::unique_ptr<UndoRedo::IPublic> CreateInterface()
	{
		return CreateInterface<VW>();
	}

}
#pragma endregion


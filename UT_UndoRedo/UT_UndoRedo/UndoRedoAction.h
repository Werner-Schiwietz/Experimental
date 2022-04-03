#pragma once
//Copyright (c) 2020 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

/// Undo-Redo doing with need C++17 (if constexpr)
/// usage see unittest UndoRedo.cpp
/// 

#include <functional>
#include <memory>
#include <deque>
#include <string>

#ifndef UndoRedo_StringType
	#define UndoRedo_StringType std::wstring
#endif
#pragma region public interface
namespace UndoRedo
{
	enum class Direction;
	using string_t = UndoRedo_StringType;

#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ = 0
	struct IDoingText
	{
		using string_t = UndoRedo::string_t;
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

		#pragma region adding
			virtual void Add( action_t action, action_t undo ) _INTERFACE_FUNCTION_;
			virtual void Add( action_t action, action_t undo, std::shared_ptr<IDoingText>) _INTERFACE_FUNCTION_;
			virtual void AddAndDo( action_t action, action_t undo ) _INTERFACE_FUNCTION_;
			virtual void AddAndDo( action_t action, action_t undo, std::shared_ptr<IDoingText>) _INTERFACE_FUNCTION_;
		#pragma endregion new action

		#pragma region execute Undo-Redo-action
			//return-value false -> no action avaiable
			virtual bool Undo() _INTERFACE_FUNCTION_;
			virtual bool Redo() _INTERFACE_FUNCTION_;
		#pragma endregion Undo-Redo-action

		#pragma region UserInterface strings
			//next action at index 0
			virtual textcontainer_t UndoTexte() const  _INTERFACE_FUNCTION_;
			virtual textcontainer_t RedoTexte() const  _INTERFACE_FUNCTION_;
		#pragma endregion UserInterface strings

	};
#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ override

	class VW;//normal, bei neuer action wird ReDo-Puffer geleert
	class VWHoldAllRedos;//extented, ReDo-Puffer wird auf UnDo-Puffer übertragen
	std::unique_ptr<UndoRedo::IPublic> CreateInterface();//benutzt VW, also same as UndoRedo::CreateInterface<VW>()
	template<typename IPublic_Impl_t> std::unique_ptr<UndoRedo::IPublic> CreateInterface();//usage auto IPublicPtr = UndoRedo::CreateInterface<VWHoldAllRedos>();
}
#pragma endregion


#pragma region definition 
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
			static string_t const textUndo{UndoText<string_t>()};
			static string_t const textRedo{RedoText<string_t>()};

			switch(direction)
			{
			case Direction::Undo:
				return textUndo;
			case Direction::Redo:
				return textRedo;
			}
			throw std::invalid_argument( __FUNCSIG__ " direction unknown " );
		}
		#pragma region string_helper
		template<typename string_type> static string_type UndoText()
		{
			if constexpr ( std::is_constructible_v<string_type,char const *> )
				return "Undo";
			else if constexpr ( std::is_constructible_v<string_type,wchar_t const *> )
				return L"Undo";
			else
				static_assert(false,"what?");
		}
		template<typename string_type> static string_type RedoText()
		{
			if constexpr ( std::is_constructible_v<string_type,char const *> )
				return "Redo";
			else if constexpr ( std::is_constructible_v<string_type,wchar_t const *> )
				return L"Redo";
			else
				static_assert(false,"what?");
		}
		#pragma endregion 
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

	/// <summary>
	/// IPublic_StdImpl vollständige Implementierung von  IPublic
	/// es muss noch die pure virtual function handle_redos implementiert werden. Siehe VW und VWHoldAllRedos
	/// </summary>
	class IPublic_StdImpl : public IPublic
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
		virtual void			_handle_redos() = 0;
	#pragma endregion 
	};
	/// <summary>
	/// VW. beim Add einer Action wird der ReDo-Puffer geleert
	/// </summary>
	class VW : public IPublic_StdImpl
	{
	private:
		virtual void			_handle_redos() override;//umkopieren von Redo-Stack auf den Undo-Stack, statt clear()
	};
	/// <summary>
	/// VWHoldAllRedos beim Add einer Action wird der ReDo-Puffer auf den UnDo-Puffer übertragen
	/// </summary>
	class VWHoldAllRedos : public IPublic_StdImpl
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
	inline void											IPublic_StdImpl::Action::Invoke() 
	{
		if( this->direction == Direction::Redo )
			redo_action();
		else
			undo_action();
	}
	inline IPublic_StdImpl::Action						IPublic_StdImpl::Action::Toggle() &&	
	{
		this->direction = UndoRedo::Toggle(this->direction);
		return std::move( *this );
	}
	inline IPublic_StdImpl::Action						IPublic_StdImpl::Action::MoveFromRedoToUndo() const &	
	{
		return *this;//just copy, used by VWHoldAllRedos::_handle_redos
	}
	inline IPublic_StdImpl::Action::string_t const &	IPublic_StdImpl::Action::DoingText() const
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

	inline bool IPublic_StdImpl::_action( Stack & from, Stack & to )//static
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
	inline bool IPublic_StdImpl::Undo()
	{
		return _action( this->undos, this->redos );
	}
	inline bool IPublic_StdImpl::Redo()
	{
		return _action( this->redos, this->undos );
	}

	inline void IPublic_StdImpl::Add( action_t action, action_t undo, std::shared_ptr<IDoingText> doingtext )
	{
		_handle_redos();
		undos.emplace( undo, action, doingtext );
	}
	inline void IPublic_StdImpl::Add( action_t action, action_t undo)
	{
		_handle_redos();
		undos.emplace( undo, action );
	}
	inline void IPublic_StdImpl::AddAndDo( action_t action, action_t undo, std::shared_ptr<IDoingText> doingtext )
	{
		action();
		Add( action, undo, doingtext );
	}
	inline void IPublic_StdImpl::AddAndDo( action_t action, action_t undo)
	{
		action();
		Add( action, undo );
	}

	inline IPublic_StdImpl::textcontainer_t IPublic_StdImpl::_Texte( Stack const & data )//static
	{
		IPublic_StdImpl::textcontainer_t ret_value;
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
#pragma endregion of public interface


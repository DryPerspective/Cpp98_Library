#ifndef DP_CPP98_BORLAND_RAII
#define DP_CPP98_BORLAND_RAII

#ifndef __BORLANDC__
#error "This header requires a Borland/C++Builder compiler."
#else

/*
*   A collection of tools which exploit the RAII pattern to achieve a particular goal.
*	These tools assume TEMPORARY ownership of a particular resource for a given scope. They do not manage lifetimes,
*   they are not smart pointers. They simply allow a single scope block to ensure that a particular action is
*   always performed upon exit from that block, however it goes.
*	Not smart pointers because C++Builder runs on a world of global objects with static lifetime. These tools
*   won't create or destroy them, but allows a function to do necessary cleanup automatically on all paths.
*/


#include <vcl.h>	
#include <Data.Win.ADODB.hpp>
//#include "QRPDFFilt.hpp"



namespace dp {
namespace raii {

	namespace detail {
		//A common base class which will inhibit the compiler from generating copy constructors
		//for any class which inherits from it. Keeps things DRY since we need it so much here.
		//I can't hide things in a header, so you should pinky promise not to use this for external things
		class disable_copies {
		protected:
			disable_copies() {}
			~disable_copies() {}
		private:
			disable_copies(const disable_copies&);
			disable_copies& operator=(const disable_copies&);
		};
	}

	//A class to disable a component and reenable it at the end of the scope block
	class disabler : detail::disable_copies {
		Vcl::Controls::TControl* comp;

	public:
		disabler(TControl* in) : comp(in) {
			if(comp) comp->Enabled = false;
		}
		~disabler() {
			if(comp) comp->Enabled = true;
		}
	};

	//Set the cursor to the input value, and automatically return it to the previous type
	//e.g. Creating a raii::cursor(crHourGlass) when the screen cursor is crDefault
	//will set the cursor to the hourglass and automatically turn it back to crDefault at the end of the scope
	class cursor : detail::disable_copies {
		System::Uitypes::TCursor cursor;

	public:
		cursor(System::Uitypes::TCursor SetTo) : cursor(Screen->Cursor) {
			Screen->Cursor = SetTo;
		}
		~cursor() {
			Screen->Cursor = this->cursor;
		}

	};

	//A "query holder" which can take function-local ownership of a particular TADOQuery
	//and will automatically clear and close the query on destruction.
	//Intended usage is to start the function with something like raii:query qry(DMod->WhateverQry); and then use
	//qry to access the query from there on out.
	//It *should* work if you just construct it at the beginning (with a named variable) and then refer back to the original
	//DMod->Whateverqry in the function. Should. I make no promises or guarantees in the event of misuse.
	class query : detail::disable_copies {

		typedef Data::Win::Adodb::TADOQuery qry_t;

		qry_t* qry;

	public:
		query(qry_t* in) : qry(in) {}
		~query() {
			if (qry) {
				qry->Close();
				qry->SQL->Clear();
				qry->Parameters->Clear();
			}
		}

		qry_t* get() {
			return qry;
		}
		const qry_t* get() const {
			return qry;
		}

		qry_t* operator->() {
			return qry;
		}
		const qry_t* operator->() const {
			return qry;
		}

		qry_t& operator*() {
			return *qry;
		}
		const qry_t& operator*() const {
			return *qry;
		}
	};


	//A holder for a TADOConnection object to automatically rollback any ongoing transaction in the event
	//of an abnormal end to the function
	class connection : detail::disable_copies {
		typedef Data::Win::Adodb::TADOConnection conn_t;

		conn_t* conn;

	public:
		connection(conn_t* in) : conn(in) {}
		~connection() {
			if (conn && conn->InTransaction) conn->RollbackTrans();
		}

		conn_t* get() {
			return conn;
		}
		const conn_t* get() const {
			return conn;
		}

		conn_t* operator->() {
			return conn;
		}
		const conn_t* operator->() const {
			return conn;
		}

		conn_t& operator*() {
			return *conn;
		}
		const conn_t& operator*() const {
			return *conn;
		}
	};



}
}

#endif	//Ifndef borland
#endif	//Header guard
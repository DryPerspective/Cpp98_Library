#ifndef DP_CPP98_BORLAND_DELETERS
#define DP_CPP98_BORLAND_DELETERS

#ifndef __BORLANDC__
#error "This header requires a Borland/C++Builder compiler."
#else

#include <vcl.h>
#include <Data.Win.ADODB.hpp>

namespace dp {
	/*
	*	A couple of common deleters, intended for use with smart pointers and a particular flavor of query object which must be dynamically allocated.
	*/
	struct query_deleter {
		void operator()(Data::Win::Adodb::TADOQuery* qry) {
			qry->Close();
			qry->SQL->Clear();
			qry->Parameters->Clear();
			delete qry;
		}
	};

	struct connection_deleter {
		void operator()(Data::Win::Adodb::TADOConnection* conn){
			if (conn->InTransaction) conn->RollbackTrans();
			delete conn;
		}
	};

}


#endif  //ifndef BORLAND
#endif  //header guard
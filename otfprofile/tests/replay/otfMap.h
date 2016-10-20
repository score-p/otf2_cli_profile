#ifndef OTFMAP_H
#define OTFMAP_H

#include "otf.h"
#include <map>
#include <exception>
#include <string>
#include <sstream>

#include <mpi.h>

#include "replayExceptions.h"
#include "otfListWrapper.h"



template<class T>
class OtfMap : public std::map<uint32_t, T>{
	
public:
	
	T* get_ptr_from_list( OTF_KeyValueList *list, uint32_t id ) throw( OtfNotFoundException ){
		int ret;
		uint32_t value_id;
		ret = OTF_KeyValueList_getUint32( list, id, &value_id );
	
		if ( ret != 0 )
		{
			throw OtfListKeyNotFoundException( id );
		} else if( ! this->contains( value_id ) )
		{
			throw OtfMapKeyNotFoundException( value_id );
		}
		return &(*this)[ value_id ];
	}

	bool contains( uint32_t id ){ return ( this->find( id ) != this->end() ); }

	T get_from_list( OTF_KeyValueList *list, uint32_t id ) throw( OtfNotFoundException ){
		return *get_ptr_from_list( list, id );
	}

	T get_by_id( uint32_t id ){
		if ( ! this->contains( id ) )
		{
			throw OtfMapKeyNotFoundException( id );
		}
		return (*this)[ id ];
	}

};

#endif /* OTFMAP_H */

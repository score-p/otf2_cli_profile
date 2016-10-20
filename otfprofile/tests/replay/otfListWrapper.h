#ifndef OTFLISTWRAPPER_H
#define OTFLISTWRAPPER_H


#include <otf.h>

class OtfKVListWrapper{
private:
	OTF_KeyValueList* _list;

public:
	void setList( OTF_KeyValueList* list ){ this->_list = list; }
	OTF_KeyValueList* getList( void ){ return this->_list; }
	
	uint32_t getUint32( uint32_t key );
	uint8_t getUint8( uint32_t key );

};


#endif /* OTFLISTWRAPPER_H */

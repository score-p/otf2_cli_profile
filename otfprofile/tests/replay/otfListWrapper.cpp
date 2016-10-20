#include <otf.h>
#include "otfListWrapper.h"
#include "otfMap.h"

uint32_t OtfKVListWrapper::getUint32( uint32_t key )
{
	uint32_t result;
	uint8_t ret;
	ret = OTF_KeyValueList_getUint32( _list, key, &result );
	if ( ret != 0 )
	{
		throw OtfListKeyNotFoundException( key );
	}
	return result;
}

uint8_t OtfKVListWrapper::getUint8( uint32_t key )
{
	uint8_t result;
	uint8_t ret;
	ret = OTF_KeyValueList_getUint8( _list, key, &result );
	if ( ret != 0 )
	{
		throw OtfListKeyNotFoundException( key );
	}
	return result;
}


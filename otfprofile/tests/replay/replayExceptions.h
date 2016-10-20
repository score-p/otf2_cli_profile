
#include <exception>
#include <string>
#include <sstream>

#ifndef REPLAY_EXCEPTIONS_H
#define REPLAY_EXCEPTIONS_H

class OtfNotFoundException : public std::exception{
private:
	uint32_t key;
	
protected:
	virtual std::string mesg( void ) const { return "Unknown ID: "; }
	
public:
	OtfNotFoundException( uint32_t key ){
		this->key = key;
	}

	~OtfNotFoundException() throw(){ }

	const char* what() const throw()
	{
		std::stringstream sstr;
		sstr << mesg() << "'" << this->key << "'!\n";
		return sstr.str().c_str();
	}
};

class OtfListKeyNotFoundException : public OtfNotFoundException {
protected:
	std::string mesg( void ) const {
		return "Key not found in OTFKeyValueList: ";
	}
public:
	OtfListKeyNotFoundException( uint32_t key ) : OtfNotFoundException( key ){	}
};

class OtfMapKeyNotFoundException : public OtfNotFoundException {
protected:
	std::string mesg( void ) const {
		return "Key not found in Map: ";
	}
public:
	OtfMapKeyNotFoundException( uint32_t key ) : OtfNotFoundException( key ){	}
};

#endif /* REPLAY_EXCEPTIONS_H */

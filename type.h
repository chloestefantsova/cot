#ifndef COT_TYPE_H
#define COT_TYPE_H

// Classes containing the information about field types are held
// withing this file. The file is under development.

#include <cstring>
#include <string>
#include <stdint.h>
#include <mysql/mysql.h>
#include "exception.h"

class AbstractValue
{
    public:
        ~AbstractValue() {}
        
        virtual int get_mysql_in_type_code() const = 0;
        virtual int get_mysql_out_type_code() const = 0;
        virtual int get_max_length() const = 0;
        
        virtual void bind(void *, int) = 0;
        virtual bool is_bound() { return bound_; }
        virtual void unbind() { bound_ = false; }
        
        virtual void * get_bind_address() const { return bind_address_; }
        virtual void * get_data_address() const { return data_address_; }
        virtual int get_bind_length() const = 0;
        virtual int get_data_length() const = 0;
        virtual int get_max_data_length() const = 0;
        
    protected:
        void * bind_address_;
        void * data_address_;
        bool bound_;
};

template<class __CType, class __CppType, int __MysqlInType, int __MysqlOutType, int __MaxLength>
class Value: public AbstractValue
{
    public:
        typedef __CType ctype;
        typedef __CppType cpptype;
        static const int mysql_in_type_code = __MysqlInType;
        static const int mysql_out_type_code = __MysqlOutType;
        static const int max_length = __MaxLength;
        
        virtual ~Value() {}
        
        virtual int get_mysql_in_type_code() const {
            return mysql_in_type_code;
        }
        
        virtual int get_mysql_out_type_code() const {
            return mysql_out_type_code;
        }
        
        virtual int get_max_length() const {
            return max_length;
        }
};

#define GET_CPP_REPR(valuetype, value) \
        ((typename valuetype::cpptype)(*(typename valuetype::ctype *)(value->get_bind_address())))

class IntValue: public Value<int, int, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, sizeof(int)>
{
    public:
        static IntValue * from_ctype(int value) {
            IntValue * result = new IntValue;
            result->value_ = value;
            return result;
        }
        static IntValue * from_cpptype(int value) {
            IntValue * result = new IntValue;
            result->value_ = value;
            return result;
        }
        static IntValue * from_dump(void * mem) {
            IntValue * result = new IntValue;
            result->bind_address_ = mem;
            result->data_address_ = mem;
            result->value_ = *(int *)result->data_address_;
            result->bound_ = true;
            return result;
        }
        
        IntValue() {
            value_ = 0;
            bound_ = false;
        }
        
        virtual ~IntValue() {}
        
        virtual void bind(void * mem, int bytes_available) {
            if (bound_) {
                throw new CotException("bind(): IntValue instance already bound");
            }
            if (bytes_available < get_bind_length()) {
                throw new CotException("bind(): not enough memory passed to bind IntValue instance");
            }
            bind_address_ = mem;
            data_address_ = mem;
            *(int *)data_address_ = value_;
            bound_ = true;
        }
        
        virtual int get_bind_length() const {
            return max_length;
        }
        
        virtual int get_data_length() const {
            return max_length;
        }
        
        virtual int get_max_data_length() const {
            return max_length;
        }
        
    protected:
        int value_;
};

// The auxiliary data includes null byte for C-string and the pointer
// to the begining of the actual string (in fact, the next byte after the
// pointer).

template<int __Length>
class StringValue: public Value<
        char *,
        std::string,
        MYSQL_TYPE_STRING,
        MYSQL_TYPE_VAR_STRING,
        sizeof(char *) + __Length + 1>
{
    public:
        static StringValue<__Length> * from_ctype(char * value) {
            StringValue<__Length> * result = new StringValue<__Length>;
            result->value_ = std::string(value);
            if ((int)result->value_.length() > StringValue<__Length>::max_length) {
                throw new CotException("from_ctype(): "
                    "the passed value length is greater than the StringValue max_length parameter");
            }
            return result;
        }
        static StringValue<__Length> * from_cpptype(std::string value) {
            StringValue<__Length> * result = new StringValue<__Length>;
            result->value_ = value;
            if ((int)result->value_.length() > StringValue<__Length>::max_length) {
                throw new CotException("from_cpptype(): "
                    "the passed value length is greater than the StringValue max_length parameter");
            }
            return result;
        }
        static StringValue<__Length> * from_dump(void * mem) {
            StringValue<__Length> * result = new StringValue<__Length>;
            result->bind_address_ = mem;
            result->data_address_ = (uint8_t *)mem + sizeof(char *);
            result->value_ = std::string((char *)result->data_address_);
            if ((int)result->value_.length() > StringValue<__Length>::max_length) {
                throw new CotException("from_dump(): "
                    "the passed value length is greater than the StringValue max_length parameter");
            }
            result->bound_ = true;
            return result;
        }
        
        StringValue() {
            value_ = "";
            this->bound_ = false;
        }
        
        ~StringValue() {}
        
        virtual void bind(void * mem, int bytes_available) {
            if (this->bound_) {
                throw new CotException("bind(): the StringValue instance is already bound");
            }
            if (bytes_available < get_bind_length()) {
                throw new CotException("bind(): not enough memory passed to bind the StringValue instance");
            }
            this->bind_address_ = mem;
            this->data_address_ = (uint8_t *)mem + sizeof(char *);
            *(char **)this->bind_address_ = (char *)this->data_address_;
            std::strncpy((char *)this->data_address_, value_.c_str(), get_data_length() + 1);
            this->bound_ = true;
        }
        
        virtual int get_bind_length() const {
            return StringValue<__Length>::max_length;
        }
        
        virtual int get_data_length() const {
            return value_.length();
        }
        
        virtual int get_max_data_length() const {
            return __Length;
        }
    
    protected:
        std::string value_;
};

#endif // COT_TYPE_H


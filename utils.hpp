#ifndef UTILS_H
#define UTILS_H

#include<string>
#include<vector>
#include<iostream>
#include<fstream>
#include<array>
#include<functional>
#include<stdexcept>
#include<map>
#include<queue>
#include<stack>
#include<memory>

class Token{
    private:
    std::string id;
    std::string val;
    std::pair<int,int> pos;

    public:
    Token();
    Token(std::string x, int lineNo, int charNo);
    Token(std::string x, std::string y, int lineNo, int charNo);
    friend std::ostream& operator<<(std::ostream& os, Token const& t);
    std::string getId();
    std::string getVal();
    std::pair<int,int> getPos();
};

bool isValToken(std::string s);
std::vector<std::string> split(std::string s, std::string t);
std::string charFormat(char c);
std::string regFormat(std::string s, std::string name);
int numChar(int val);

namespace types {
    enum TYPES {
        UNRESOLVED,
        BOOL,
        CHAR_UNRESOLVED,
        CHAR_8,
        INT_UNRESOLVED,
        INT_8,
        INT_16,
        INT_32,
        INT_64,
        INT_128,
        UINT_UNRESOLVED,
        UINT_8,
        UINT_16,
        UINT_32,
        UINT_64,
        UINT_128,
        FLOAT_UNRESOLVED,
        FLOAT_8,
        FLOAT_16,
        FLOAT_32,
        FLOAT_64,
        FLOAT_128,
        FUNCTION
    };

    enum MAX_BITS {
        B8 = 8,
        B16 = 16,
        B32 = 32,
        B64 = 64,
        B128 = 128
    };

    template<MAX_BITS mb>
    struct mInt {
        static const TYPES dt = INT_UNRESOLVED;
    };

    template<>
    struct mInt<B8> {
        static const TYPES dt = INT_8;
    };

    template<>
    struct mInt<B16> {
        static const TYPES dt = INT_16;
    };

    template<>
    struct mInt<B32> {
        static const TYPES dt = INT_32;
    };

    template<>
    struct mInt<B64> {
        static const TYPES dt = INT_64;
    };

    template<>
    struct mInt<B128> {
        static const TYPES dt = INT_128;
    };

    /* Signed integers are defined by using the templated `Integer` class which also takes in the number
    of bits used to store the integer. Type aliases for specific bit sizes have been made as `i8` all
    the way to `i128` at powers of 2.
    */
    template<MAX_BITS bitSize>
    class Integer {
        static_assert(bitSize%8==0, "Bit Size for class Integer must be a multiple of 8");
        static_assert(bitSize>0, "Bit Size for class Integer must be a positive integer");
        // Values as stored in the Little Endian System
        unsigned char value[bitSize/8];

    public:
        static constexpr TYPES dt = mInt<bitSize>::dt;
        
        inline Integer() {
            for(unsigned char& b : value)
                b = 0;
        }

        inline Integer(std::vector<unsigned char>& value) {
            for(unsigned short int i = 0, bytes = bitSize/8; i < bytes; i++)
                this->value[i] = value[i];
        }
    };

    typedef Integer<B8> i8;
    typedef Integer<B16> i16;
    typedef Integer<B32> i32;
    typedef Integer<B64> i64;
    typedef Integer<B128> i128;

    /* Unsigned integers are defined by using the templated `UnsignedInteger` class which also takes in 
    the number of bits used to store the unsigned integer as a template parameter. Type aliases for 
    specific bit sizes have been made as `u8` all the way to `u128` at powers of 2.
    */
    template<MAX_BITS bitSize>
    class UnsignedInteger {
        static_assert(bitSize%8==0, "Bit Size for class UnsignedInteger must be a multiple of 8");
        static_assert(bitSize>0, "Bit Size for class UnsignedInteger must be a positive integer");
        // Values as stored in the Little Endian System
        unsigned char value[bitSize/8];

    public:
        inline UnsignedInteger() {
            for(unsigned char& b : value)
                b = 0;
        }

        inline UnsignedInteger(std::vector<unsigned char>& value) {
            for(unsigned short int i = 0, bytes = bitSize/8; i < bytes; i++)
                this->value[i] = value[i];
        }
    };

    template<>
    class UnsignedInteger<B8> {
        static const TYPES dt = INT_8;
    };

    template<>
    class UnsignedInteger<B16> {
        static const TYPES dt = INT_16;
    };

    template<>
    class UnsignedInteger<B32> {
        static const TYPES dt = INT_32;
    };

    template<>
    class UnsignedInteger<B64> {
        static const TYPES dt = INT_64;
    };

    template<>
    class UnsignedInteger<B128> {
        static const TYPES dt = INT_128;
    };

    typedef UnsignedInteger<B8> u8;
    typedef UnsignedInteger<B16> u16;
    typedef UnsignedInteger<B32> u32;
    typedef UnsignedInteger<B64> u64;
    typedef UnsignedInteger<B128> u128;

    template<MAX_BITS bitSize>
    class Float {
        static_assert(bitSize%8==0, "Bit Size for class Float must be a multiple of 8");
        static_assert(bitSize>0, "Bit Size for class Float must be a positive integer");
        // Values as stored in the Little Endian System
        unsigned char value[bitSize/8];

    public:
        inline Float() {
            for(unsigned char& b : value)
                b = 0;
        }
    };

    typedef Float<B8> f8;
    typedef Float<B16> f16;
    typedef Float<B32> f32;
    typedef Float<B64> f64;
    typedef Float<B128> f128;

    class Boolean {
        unsigned char value;

    public:
        inline Boolean() : value(0) {}
    };
//    c8 // UTF 8
//    c16 // UTF 16 --
}

// Both the functions below return an array of bytes
namespace utils{
    std::pair<types::MAX_BITS, std::vector<unsigned char>> stringToNumberUtil(std::string& source);
}

// This function always returns a 64-bit float, later, this float is cast down as needed
namespace utils{
    std::pair<types::MAX_BITS, std::vector<unsigned char>> stringToFloatUtil(std::string& source);
}

#endif
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

#define LOG(x) // std::cout<<x<<std::endl;

class Token {
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

namespace ast {
    enum NodeType {
        DEF_AST,
        PROG_AST,
        BLOCK_AST,
        LOOP_AST,
        COND_AST,
        RET_AST,
        LOG_AST,
        LETFUN_AST,
        LETARR_AST,
        LETVAR_AST,
        LETCONST_AST,
        ASSIGN_AST,
        BOP_AST,
        UOP_AST,
        FUNCALL_AST,
        ARR_AST,
        NUM_AST,
        CHAR_AST,
        BOOL_AST,
        NULL_AST,
        IDEN_AST,
    };

    class ASTNode {
    public:

        virtual NodeType type() const {
            return DEF_AST;
        }

        /*virtual AtomicASTNode& const_resolve() {
            // unimplemented
        }*/
    };
}

namespace types {

    enum TYPES {
        UNKNOWN,
        ATOM,
        PTR,
        FN,
        ARR,
        ARRD
    };
    
    enum ATOMTYPES {
        UNRESOLVED,
        ANY,
        NULLV,
        BOOL,
        CHAR_8,
        NUM,
        INT,
        INT_8,
        INT_16,
        INT_32,
        INT_64,
        INT_128,
        UINT,
        UINT_8,
        UINT_16,
        UINT_32,
        UINT_64,
        UINT_128,
        FLOAT,
        FLOAT_16,
        FLOAT_32,
        FLOAT_64,
        FLOAT_128
    };

    enum MAX_BITS {
        B8 = 8,
        B16 = 16,
        B32 = 32,
        B64 = 64,
        B128 = 128
    };

    class Type {
    public:
        virtual TYPES name() const {
            return UNKNOWN;
        };

        bool operator==(const Type& other) const;
    };

    std::unique_ptr<Type> gtc(Type& other); // global type copy

    class DeclType : public Type {};

    class CompoundType : public Type {};

    class AtomicType : public DeclType {
    public:
        TYPES name() const override {
            return ATOM;
        }

        virtual ATOMTYPES atomicName() const = 0;

        bool operator==(const AtomicType& other) const {
            return this->atomicName() == other.atomicName();
        }
    };

    class NumberType : public AtomicType {
    public:
        ATOMTYPES atomicName() const override {
            return NUM;
        }
    };

    template<MAX_BITS mb>
    struct mInt {
        static const ATOMTYPES dt = INT;
    };

    template<>
    struct mInt<B8> {
        static const ATOMTYPES dt = INT_8;
    };

    template<>
    struct mInt<B16> {
        static const ATOMTYPES dt = INT_16;
    };

    template<>
    struct mInt<B32> {
        static const ATOMTYPES dt = INT_32;
    };

    template<>
    struct mInt<B64> {
        static const ATOMTYPES dt = INT_64;
    };

    template<>
    struct mInt<B128> {
        static const ATOMTYPES dt = INT_128;
    };

    /* Signed integers are defined by using the templated `Integer` class which also takes in the number
    of bits used to store the integer. Type aliases for specific bit sizes have been made as `i8` all
    the way to `i128` at powers of 2.
    */
    template<MAX_BITS bitSize>
    class Integer : public NumberType {
        static_assert(bitSize%8==0, "Bit Size for class Integer must be a multiple of 8");
        static_assert(bitSize>0, "Bit Size for class Integer must be a positive integer");
        // Values as stored in the Little Endian System
    public:
        unsigned char value[bitSize/8];

        static constexpr ATOMTYPES dt = mInt<bitSize>::dt;
        
        inline Integer() {
            for(unsigned char& b : value)
                b = 0;
        }

        inline Integer(std::vector<unsigned char>& value) {
            for(unsigned short int i = 0, bytes = bitSize/8; i < bytes; i++)
                this->value[i] = value[i];
        }

        Integer(Integer<bitSize>& other) {
            LOG("inside integer constructor (as expected)")
            for(unsigned short int byte = 0; byte < bitSize/8; byte++)
                value[byte] = other.value[byte];
        }

        ATOMTYPES atomicName() const override {
            return dt;
        }
    };

    typedef Integer<B8> i8;
    typedef Integer<B16> i16;
    typedef Integer<B32> i32;
    typedef Integer<B64> i64;
    typedef Integer<B128> i128;

    template<MAX_BITS mb>
    struct mUInt {
        static const ATOMTYPES dt = UINT;
    };

    template<>
    struct mUInt<B8> {
        static const ATOMTYPES dt = UINT_8;
    };

    template<>
    struct mUInt<B16> {
        static const ATOMTYPES dt = UINT_16;
    };

    template<>
    struct mUInt<B32> {
        static const ATOMTYPES dt = UINT_32;
    };

    template<>
    struct mUInt<B64> {
        static const ATOMTYPES dt = UINT_64;
    };

    template<>
    struct mUInt<B128> {
        static const ATOMTYPES dt = UINT_128;
    };
    
    /* Unsigned integers are defined by using the templated `UnsignedInteger` class which also takes in 
    the number of bits used to store the unsigned integer as a template parameter. Type aliases for 
    specific bit sizes have been made as `u8` all the way to `u128` at powers of 2.
    */
    template<MAX_BITS bitSize>
    class UnsignedInteger : public NumberType {
        static_assert(bitSize%8==0, "Bit Size for class UnsignedInteger must be a multiple of 8");
        static_assert(bitSize>0, "Bit Size for class UnsignedInteger must be a positive integer");
        // Values as stored in the Little Endian System
    public:
        unsigned char value[bitSize/8];

        static constexpr ATOMTYPES dt = mUInt<bitSize>::dt;

        inline UnsignedInteger() {
            for(unsigned char& b : value)
                b = 0;
        }

        inline UnsignedInteger(std::vector<unsigned char>& value) {
            for(unsigned short int i = 0, bytes = bitSize/8; i < bytes; i++)
                this->value[i] = value[i];
        }
    
        UnsignedInteger(UnsignedInteger<bitSize>& other) {
            for(unsigned short int byte = 0; byte < bitSize/8; byte++)
                value[byte] = other.value[byte];
        }

        ATOMTYPES atomicName() const override {
            return dt;
        }
    };

    typedef UnsignedInteger<B8> u8;
    typedef UnsignedInteger<B16> u16;
    typedef UnsignedInteger<B32> u32;
    typedef UnsignedInteger<B64> u64;
    typedef UnsignedInteger<B128> u128;

    template<MAX_BITS mb>
    struct mFloat {
        static const ATOMTYPES dt = FLOAT;
    };

    template<>
    struct mFloat<B16> {
        static const ATOMTYPES dt = FLOAT_16;
    };

    template<>
    struct mFloat<B32> {
        static const ATOMTYPES dt = FLOAT_32;
    };

    template<>
    struct mFloat<B64> {
        static const ATOMTYPES dt = FLOAT_64;
    };

    template<>
    struct mFloat<B128> {
        static const ATOMTYPES dt = FLOAT_128;
    };

    template<MAX_BITS bitSize>
    class Float : public NumberType {
        static_assert(bitSize%8==0, "Bit Size for class Float must be a multiple of 8");
        static_assert(bitSize>0, "Bit Size for class Float must be a positive integer");

        // Values as stored in the Little Endian System
    public:
        unsigned char value[bitSize/8];

        static constexpr ATOMTYPES dt = mFloat<bitSize>::dt;

        inline Float() {
            static_assert(bitSize==B32 or bitSize==B64, "Only single and double point precision floats allowed right now.");
            for(unsigned char& b : value)
                b = 0;
        }

        inline Float(std::vector<unsigned char>& value) {
            static_assert(bitSize==B32 or bitSize==B64, "Only single and double point precision floats allowed right now.");
            for(unsigned short int i = 0, bytes = bitSize/8; i < bytes; i++)
                this->value[i] = value[i];
        }

        Float(Float<bitSize>& other) {
            for(unsigned short int byte = 0; byte < bitSize/8; byte++)
                value[byte] = other.value[byte];
        }

        ATOMTYPES atomicName() const override {
            return dt;
        }
    };

    typedef Float<B16> f16;
    typedef Float<B32> f32;
    typedef Float<B64> f64;
    typedef Float<B128> f128;

    class Boolean : public AtomicType {
    public:
        unsigned char value;

        inline Boolean() : value(0) {}

        inline Boolean(bool v) {
            if(v)
                value = 1;
            else
                value = 0;
        }

        inline Boolean(Boolean& other) : value(other.value) {}

        ATOMTYPES atomicName() const override {
            return BOOL;
        }
    };

    class Character : public AtomicType {
    public:
        unsigned char value;

        inline Character() : value(0) {}

        inline Character(unsigned char c) : value(c) {}

        inline Character(Character& other) : value(other.value) {}

        ATOMTYPES atomicName() const override {
            return CHAR_8;
        }
    };

    typedef Character c8;

    class Null : public AtomicType {
    public:
        ATOMTYPES atomicName() const override {
            return NULLV;
        }
    };

    class PointerDeclType : public DeclType {
    public:
        std::unique_ptr<Type> underlyingType;

        TYPES name() const override {
            return PTR;
        }

        PointerDeclType(std::unique_ptr<Type>&& ut) : underlyingType(std::move(ut)) {}

        PointerDeclType(PointerDeclType& other) {
            this->underlyingType = gtc(*(other.underlyingType));
        }

        bool operator==(const PointerDeclType& other) const {
            if(this->underlyingType->name() == ATOM) {

            }
            else if(other.name() == ATOM)
            return *(this->underlyingType) == *(other.underlyingType);
        }
    };

    class PointerType : public CompoundType {
    public:
        std::unique_ptr<Type> underlyingType;
        TYPES name() const override {
            return PTR;
        }

        PointerType(std::unique_ptr<Type>&& ut) : underlyingType(std::move(ut)) {}

        PointerType(PointerType& other) {
            this->underlyingType = gtc(*(other.underlyingType));
        }

        bool operator==(const PointerType& other) const {
            return *(this->underlyingType) == *(other.underlyingType);
        }
    };

    class FunctionDeclType : public DeclType {
    public:
        std::unique_ptr<Type> returnType;
        std::vector<std::unique_ptr<Type>> paramTypes;
        TYPES name() const override {
            return FN;
        }

        FunctionDeclType(std::unique_ptr<Type>&& rt, std::vector<std::unique_ptr<Type>>&& pt) : returnType(std::move(rt)), paramTypes(std::move(pt)) {}
        FunctionDeclType(std::unique_ptr<Type>&& rt) : returnType(std::move(rt)), paramTypes(std::vector<std::unique_ptr<Type>>()) {}
        FunctionDeclType(std::vector<std::unique_ptr<Type>>&& pt) : returnType(std::make_unique<Null>()), paramTypes(std::move(pt)) {}
        FunctionDeclType() : returnType(std::make_unique<Null>()), paramTypes(std::vector<std::unique_ptr<Type>>()) {}

        FunctionDeclType(FunctionDeclType& other) {
            this->returnType = gtc(*(other.returnType));
            for(auto& param : other.paramTypes) {
                this->paramTypes.push_back(gtc(*(param)));
            }
        }

        inline void addParams(std::unique_ptr<Type>&& pt) {
            paramTypes.push_back(std::move(pt));
        }

        bool operator==(const FunctionDeclType& other) const {
            if((*returnType) == (*(other.returnType)) and paramTypes.size() == other.paramTypes.size()) {
                for(size_t i = 0; i < paramTypes.size(); i++) {
                    if((*(paramTypes[i])) == (*(other.paramTypes[i]))){}
                    else {return false;}
                }
                return true;
            }
            return false;
        }
    };

    class FunctionType : public CompoundType {
    public:
        std::unique_ptr<Type> returnType;
        std::vector<std::unique_ptr<Type>> paramTypes;
        TYPES name() const override {
            return FN;
        }

        FunctionType(std::unique_ptr<Type>&& rt, std::vector<std::unique_ptr<Type>>&& pt) : returnType(std::move(rt)), paramTypes(std::move(pt)) {}
        FunctionType(std::unique_ptr<Type>&& rt) : returnType(std::move(rt)), paramTypes(std::vector<std::unique_ptr<Type>>()) {}
        FunctionType(std::vector<std::unique_ptr<Type>>&& pt) : returnType(std::make_unique<Null>()), paramTypes(std::move(pt)) {}
        FunctionType() : returnType(std::make_unique<Null>()), paramTypes(std::vector<std::unique_ptr<Type>>()) {}

        FunctionType(FunctionType& other) {
            LOG("let's copy return type")
            this->returnType = gtc(*(other.returnType));
            LOG("return type copied")
            for(auto& param : other.paramTypes) {
                LOG("param type to be copied")
                LOG("# param type: ")
                if(param != nullptr) {LOG(param->name())}
                else {LOG("lol: nullptr as a type?")}
                this->paramTypes.push_back(gtc(*(param)));
                LOG("param copied")
            }
        }

        inline void addParams(std::unique_ptr<Type>&& pt) {
            paramTypes.push_back(std::move(pt));
        }

        bool operator==(const FunctionType& other) const {
            if((*returnType) == (*(other.returnType)) and paramTypes.size() == other.paramTypes.size()) {
                for(size_t i = 0; i < paramTypes.size(); i++) {
                    if((*(paramTypes[i])) == (*(other.paramTypes[i]))) {}
                    else {return false;}
                }
                return true;
            }
            return false;
        }
    };

    class ArrayDeclType : public DeclType {
    public:
        std::unique_ptr<Type> underlyingType;
        std::unique_ptr<ast::ASTNode> size;

        TYPES name() const override {
            return ARRD;
        }

        ArrayDeclType(std::unique_ptr<Type>&& ut, std::unique_ptr<ast::ASTNode>&& sz) : underlyingType(std::move(ut)), size(std::move(sz)) {}

        bool operator==(const ArrayDeclType& other) const {
            if(*underlyingType == *(other.underlyingType))
                return true;
            return false;
        }
    };

    class ArrayType : public CompoundType {
    public:
        std::unique_ptr<Type> underlyingType;
        uint32_t size;
        bool sizeKnown;
        TYPES name() const override {
            return ARR;
        }

        ArrayType(std::unique_ptr<Type>&& ut) : underlyingType(std::move(ut)), size(0), sizeKnown(false) {}
        ArrayType(std::unique_ptr<Type>&& ut, uint32_t sz) : underlyingType(std::move(ut)), size(sz), sizeKnown(true) {}

        ArrayType(ArrayType& other) {
            LOG("# \tinside array copy constructor")
            LOG(other.underlyingType->name())
            this->underlyingType = gtc(*(other.underlyingType));
            LOG("inside arraytype, the underlying type received is:")
            LOG(underlyingType->name())
            this->size = other.size;
            this->sizeKnown = other.sizeKnown;
        }

        ArrayType(ArrayDeclType& other) {
            if(other.underlyingType->name() == ARRD) {
                auto ulArr = dynamic_cast<ArrayDeclType*>(other.underlyingType.get());
                underlyingType = std::make_unique<ArrayType>(*ulArr);
            }
            else {
                underlyingType = gtc(*(other.underlyingType));
                LOG("inside arraytype, the underlying type received is:")
                LOG(underlyingType->name())
            }
            size = 0;
            sizeKnown = false;
        }

        bool operator==(const ArrayType& other) const {
            if(*underlyingType == *(other.underlyingType))
                return true;
            return false;
        }
    };

    class AnyType : public AtomicType {
    public:
        ATOMTYPES atomicName() const override {
            return CHAR_8;
        }  
    };
}

namespace utils{
    std::unique_ptr<types::NumberType> stringToNumberUtil(std::string& source);
    std::unique_ptr<types::Character> stringToCharUtil(const std::string& source);
}

#endif
#include "utils.hpp"
#include "ast.hpp"

Token::Token(): id("NONE"), val("") {};
Token::Token(std::string x, int lineNo, int charNo): id(x), val(""), pos({lineNo,charNo}) {};
Token::Token(std::string x, std::string y, int lineNo, int charNo): id(x), val(y), pos({lineNo,charNo}) {};

std::ostream& operator<<(std::ostream& os, Token const& t){
    return os << t.id;
}

std::string Token::getId(){
    return id;
}

std::string Token::getVal(){
    return val;
}

std::pair<int,int> Token::getPos(){
    return pos;
}

std::vector<std::string> split(std::string s, std::string t){
    std::vector<std::string> v;
    int ls = 0;
    for(int i = 0;i < s.length();i++){
        bool match = true;
        for(int j = 0;j < t.length();j++){
            match &= s[i+j] == t[j];
        }
        if(match){
            v.push_back(s.substr(ls,i-ls));
            i += t.length() - 1;
            ls = i+1;
        }
    }
    if(ls < s.length()){
        v.push_back(s.substr(ls));
    }
    return v;
}

std::string charFormat(char c){
    std::string res;
    if(c == ' '){
        res = "space character";
    }else if(c == '\n'){
        res = "new line character";
    }else if(c == '\t'){
        res = "tab character";
    }else{
        res = "`";res += c;res += "`";
    }
    return res;
}

bool isValToken(std::string s){
    return s == "DEC" || s == "OCT" || s == "HEX" || s == "BIN" || s == "BOOL" || s == "IDEN";
}

std::string regFormat(std::string s, std::string name){
    std::string res = "";
    for(int i = 0;i < s.length();i++){
        if(s[i] == '\\'){
            i++;
            if(i < s.length()){
                res += s[i];
            }
            continue;
        }
        if(s[i] == '*' || s[i] == '+' || s[i] == '-' || s[i] == '|' || s[i] == '[' || s[i] == ']'){
            return name;
        }
        res += s[i];
    }
    return res;
}

int numChar(int val){
    if(val == 0){
        return 1;
    }
    int c = 0;
    while(val != 0){
        val /= 10;
        c++;
    }
    return c;
}

std::unique_ptr<types::NumberType> utils::stringToNumberUtil(std::string& source) {

    unsigned short int bitSize = 8;
    bool isFloat = false, isUnsigned = true;
    size_t s = source.length();
    int base = 10, startPos = 0, neg = 0;
    if(s>0 and source[0]=='-') {
        neg = 1;
        isUnsigned = false;
        startPos++;
    }

    if(s-startPos>1 and source[startPos] == '0') {
        if(source[startPos+1] == 'b' or source[startPos+1] == 'B') {
            base = 2;
            startPos += 2;
        }
        else if(source[startPos+1] == 'o' or source[startPos+1] == 'O') {
            base = 8;
            startPos += 2;
        }
        else if(source[startPos+1] == 'x' or source[startPos+1] == 'X') {
            base = 16;
            startPos += 2;
        }
    }

    std::vector<unsigned char> value(1);
    
    //check for float (.)
    for(int sp = startPos; sp < s; sp++) {
        if(source[sp] == '.') {
            isFloat = true;
            isUnsigned = false;
        }
    }

    if(isFloat) {
        try {
            // interpret it as single precision float
            float res = std::stof(source);
            unsigned int resval = *(reinterpret_cast<unsigned int*>(&res));
            value.resize(4);
            for(int i = 0; i<4; i++) {
                value[i] = (unsigned char)(resval&(~((unsigned char)0)));
                resval >>= 8;
            }
            return std::make_unique<types::f32>(value);
        }
        catch(std::out_of_range) {
            try {
                // interpret it as a double precision float
                double res = std::stof(source);
                unsigned int resval = *(reinterpret_cast<unsigned int*>(&res));
                value.resize(8);
                for(int i = 0; i<8; i++) {
                    value[i] = (unsigned char)(resval&(~((unsigned char)0)));
                    resval >>= 8;
                }
                return std::make_unique<types::f64>(value);
            }
            catch(std::out_of_range) {
                throw std::logic_error("Float too big to represent in any available data type.");
            }
        }
        // reinterpret_cast<unsigned int>
    }
    else {
        if(base==16) {
            int bitPos = 0, bytePos = 0, carry = 0, newVal;
            while(startPos < s) {
                carry = 0;
                if(bytePos==(bitSize/8 - 1) and bitPos == 7) {
                    bitSize *= 2;
                }
                for(int curPos = startPos; curPos < s; curPos++) {
                    if(source[curPos]>='a' and source[curPos]<='f') {
                        newVal = (source[curPos] - 'a' + 10 + carry * base);
                        carry = newVal%2;
                        newVal /= 2;
                        if(newVal<10)
                            source[curPos] = (char)(newVal+'0');
                        else
                            source[curPos] = (char)(newVal-10+'a');
                        if(newVal == 0 and curPos == startPos)
                            startPos++;
                    }
                    else if(source[curPos] >= 'A' and source[curPos] <= 'F') {
                        newVal = (source[curPos] - 'A' + 10 + carry * base);
                        carry = newVal%2;
                        newVal /= 2;
                        if(newVal<10)
                            source[curPos] = (char)(newVal+'0');
                        else
                            source[curPos] = (char)(newVal-10+'a');
                        if(newVal == 0 and curPos == startPos)
                            startPos++;
                    }
                    else if(source[curPos]>='0' and source[curPos]<='9') {
                        newVal = (source[curPos] - '0' + carry * base);
                        carry = newVal%2;
                        newVal /= 2;
                        if(newVal<10)
                            source[curPos] = (char)(newVal+'0');
                        else
                            source[curPos] = (char)(newVal-10+'a');
                        if(newVal == 0 and curPos == startPos)
                            startPos++;
                    }
                    else {
                        // at this instant fraction or exponent must have occurred
                        break;
                    }
                }
                value[bytePos] |= (carry<<(bitPos++));
                if(bitPos==8) {
                    bytePos++;
                    value.push_back(0);
                }
                bitPos %= 8;
            }
        }
        else {
            int bitPos = 0, bytePos = 0, carry = 0, newVal;
            while(startPos < s) {
                carry = 0;
                if(bytePos==(bitSize/8 - 1) and bitPos == 7) {
                    bitSize *= 2;
                }
                for(int curPos = startPos; curPos < s; curPos++) {
                    if(source[curPos]>='0' and source[curPos]<=('0'+base-1)) {
                        newVal = (source[curPos] - '0' + carry * base);
                        carry = newVal%2;
                        newVal /= 2;
                        source[curPos] = (char)(newVal+'0');
                        if(newVal == 0 and curPos == startPos)
                            startPos++;
                    }
                    else {
                        //won't reach here
                    }
                }
                value[bytePos] |= (carry<<(bitPos++));
                if(bitPos==8) {
                    bytePos++;
                    value.push_back(0);
                }
                bitPos %= 8;
            }
        }
        
        while((int)value.size() < bitSize/8)
            value.push_back(0);

        if(!isUnsigned and value.back()&(1<<7)) {
            // the integer value itself takes the sign bit
            bitSize *= 2;
            while((int)value.size() < bitSize/8)
                value.push_back(0);
        }

        if(neg) {
            for(unsigned char& b : value) {
                b = ~b;
                if(neg) {
                    neg = (int)(b==255);
                    b++;
                }
            }
        }

        if(isUnsigned) {
            switch (bitSize) {
                case 8:
                    return std::make_unique<types::u8>(value);
                case 16:
                    return std::make_unique<types::u16>(value);
                case 32:
                    return std::make_unique<types::u32>(value);
                case 64:
                    return std::make_unique<types::u64>(value);
                case 128:
                    return std::make_unique<types::u128>(value);
                default:
                    throw std::logic_error("Unsigned Integer too big to represent in any available data type.");
            }
        }
        else {
            switch (bitSize) {
                case 8:
                    return std::make_unique<types::i8>(value);
                case 16:
                    return std::make_unique<types::i16>(value);
                case 32:
                    return std::make_unique<types::i32>(value);
                case 64:
                    return std::make_unique<types::i64>(value);
                case 128:
                    return std::make_unique<types::i128>(value);
                default:
                    throw std::logic_error("Signed Integer too big to represent in any available data type.");
            }
        }
    }
}

std::unique_ptr<types::Character> utils::stringToCharUtil(const std::string &s) {
    if(s.size() < 3 || s.front() != '\'' || s.back() != '\'')
        throw std::invalid_argument("Input must start and end with single quotes, e.g. '\\n'");
    std::string inner = s.substr(1, s.size()-2);

    if(inner.size() == 1 && inner[0] != '\\')
        return std::make_unique<types::Character>(inner[0]);

    // escape sequence
    if(inner.size() >= 2 && inner[0] == '\\'){
        char esc = inner[1];
        switch (esc) {
            case 'n':  return std::make_unique<types::Character>('\n');
            case 't':  return std::make_unique<types::Character>('\t');
            case 'r':  return std::make_unique<types::Character>('\r');
            case '\\': return std::make_unique<types::Character>('\\');
            case '\'': return std::make_unique<types::Character>('\'');
            case '\"': return std::make_unique<types::Character>('\"');
            case '0':  return std::make_unique<types::Character>('\0');
            case 'x': {
                // hex escape: \xNN
                std::string hex_digits = inner.substr(2);
                if (hex_digits.empty() || hex_digits.size() > 2 || !std::isxdigit(hex_digits[0]) || (hex_digits.size() == 2 && !std::isxdigit(hex_digits[1])))
                    break;
                int value = std::stoi(hex_digits, nullptr, 16);
                return std::make_unique<types::Character>(static_cast<unsigned char>(value));
            }
            default:
                break;
        }
    }
    throw std::invalid_argument("Unsupported or malformed escape sequence: " + s);
}
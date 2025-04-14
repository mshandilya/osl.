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

std::pair<types::MAX_BITS, std::vector<unsigned char>> utils::stringToNumberUtil(std::string& source) {
    unsigned short int bitSize = 8;
    size_t s = source.length();
    int base = 10, startPos = 0, neg = 0;
    if(s>0 and source[0]=='-') {
        neg = 1;
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
    if(base==16) {
        int bitPos = 0, bytePos = 0, carry = 0, newVal;
        while(startPos < s) {
            carry = 0;
            if(bytePos==(bitSize/8 - 1) and bitPos == 7) {
                bitSize *= 2;
            }
            for(int curPos = startPos; curPos < s; curPos++) {
                if(source[curPos]>='a' and source[curPos]<='f') {
                    newVal = (source[curPos] - 'a' + 10 + carry * base) / 2;
                    carry = newVal/base;
                    newVal %= base;
                    if(newVal<10)
                        source[curPos] = (char)(newVal+'0');
                    else
                        source[curPos] = (char)(newVal-10+'a');
                    if(newVal == 0 and curPos == startPos)
                        startPos++;
                }
                else if(source[curPos] >= 'A' and source[curPos] <= 'F') {
                    newVal = (source[curPos] - 'A' + 10 + carry * base) / 2;
                    carry = newVal/base;
                    newVal %= base;
                    if(newVal<10)
                        source[curPos] = (char)(newVal+'0');
                    else
                        source[curPos] = (char)(newVal-10+'a');
                    if(newVal == 0 and curPos == startPos)
                        startPos++;
                }
                else if(source[curPos]>='0' and source[curPos]<='9') {
                    newVal = (source[curPos] - '0' + carry * base) / 2;
                    carry = newVal/base;
                    newVal %= base;
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
                    newVal = (source[curPos] - 'a' + 10 + carry * base) / 2;
                    carry = newVal/base;
                    newVal %= base;
                    if(newVal<10)
                        source[curPos] = (char)(newVal+'0');
                    else
                        source[curPos] = (char)(newVal-10+'a');
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
    if(neg) {
        for(unsigned char& b : value) {
            b = ~b;
            if(neg) {
                neg = (int)(b==255);
                b++;
            }
        }
    }
    switch (bitSize) {
        case 8:
            return std::make_pair(types::B8, value);
        case 16:
            return std::make_pair(types::B16, value);
        case 32:
            return std::make_pair(types::B32, value);
        case 64:
            return std::make_pair(types::B64, value);
        case 128:
            return std::make_pair(types::B128, value);
        default:
            ;
            //won't reach here
    }
}
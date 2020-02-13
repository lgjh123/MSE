#include "Utils.h"
#include <codecvt>
#include <locale>
#include <iostream>

using std::wstring_convert;
using std::string;
using std::wstring;
using std::codecvt_utf8;
using std::locale;

using std::cin;
using std::cout;
using std::endl;

bool isUtf8Punct(const std::string& s) {

    wstring_convert<codecvt_utf8<wchar_t>> conv;
    wstring ws = conv.from_bytes(s);
    wchar_t ch = ws.at(0);
    locale loc("en_US.UTF-8");
    if(!ispunct(ch,loc) && !iswspace(ch)){
        return false;
    }
    return true;
}

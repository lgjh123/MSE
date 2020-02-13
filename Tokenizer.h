#pragma once
#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;

#include <set>
using std::set;

#include "cppjieba/include/cppjieba/Jieba.hpp"

class Tokenizer
{
public:
    virtual void _tokenizer(const string&,vector<cppjieba::Word>&,bool) = 0;
protected:
    static set<string> stopset;
};

class JiebaTokenizer: public Tokenizer {

public:
    JiebaTokenizer(){
        initStopSet();   //FIXME:有可能抛异常
        cout << "[ initialize ]StopWordSet "<< endl;
    }
    virtual void _tokenizer(const string&,vector<cppjieba::Word>&,bool) override;
    static void initStopSet();
    static bool isNotStop(const cppjieba::Word&);

    static const string DICT_PATH;
    static const string HMM_PATH;
    static const string USER_DICT_PATH;
    static const string IDF_PATH;
    static const string STOP_WORD_PATH;
    static cppjieba::Jieba jieba;
    static cppjieba::MPSegment seg;
};

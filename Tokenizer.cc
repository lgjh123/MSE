#include "Tokenizer.h"
#include "Utils.h"

#include <fstream>
using std::ifstream;

#include <algorithm>
using std::copy_if;

#include <iterator>
using std::back_inserter;

/*********** 静态成员初始化 ***********/
set<string> Tokenizer::stopset = set<string>();
//停用词集合

const string 
JiebaTokenizer::DICT_PATH = "cppjieba/dict/jieba.dict.utf8";
const string 
JiebaTokenizer::HMM_PATH = "cppjieba/dict/hmm_model.utf8";
const string 
JiebaTokenizer::USER_DICT_PATH = "cppjieba/dict/user.dict.utf8"; 
const string 
JiebaTokenizer::IDF_PATH = "cppjieba/dict/idf.utf8"; 
const string 
JiebaTokenizer::STOP_WORD_PATH = "cppjieba/dict/stop_words.utf8";

cppjieba::Jieba 
JiebaTokenizer::jieba = cppjieba::Jieba(DICT_PATH,
   HMM_PATH,
   USER_DICT_PATH,
   IDF_PATH,
   STOP_WORD_PATH
);

cppjieba::MPSegment     //最大概率法(MPSegment: Max Probability)分词
JiebaTokenizer::seg = cppjieba::MPSegment(JiebaTokenizer::DICT_PATH);

/*********** 静态成员初始化结束 ***********/
//初始化停用词集合
void JiebaTokenizer::initStopSet(){
    ifstream ifs(JiebaTokenizer::STOP_WORD_PATH);
    if(ifs){
        string s;
        while(ifs>>s){
            stopset.insert(s);
        }
    }
}

bool JiebaTokenizer::isNotStop(const cppjieba::Word& jiebawords){
    string word = jiebawords.word;
    if(isUtf8Punct(word)){   
        //FIXME:字典中有标点符号
        return false;
    }
    if(stopset.find(word) != stopset.end()){
        return false;
    }
    return true;
}
//分词器
void JiebaTokenizer::_tokenizer(const string& line,
                                vector<cppjieba::Word>& jiebawords,
                                bool forSearch){
    vector<cppjieba::Word> tempwords; //临时储存分词，便于筛选
    if(forSearch){      //适用于索引构建，粒度较细
        JiebaTokenizer::jieba.CutForSearch(line,tempwords);
    }
    else{
        JiebaTokenizer::seg.Cut(line,tempwords);
    }

    //过滤停用词
    if(jiebawords.size() > 0){
        jiebawords.clear();
    }
    auto inserter = back_inserter(jiebawords); //返回参数类型的迭代器
    auto it = copy_if(tempwords.begin(),tempwords.end(),inserter,isNotStop);
    //FIXME:释放多余空间
    jiebawords.shrink_to_fit();//释放多余空间
}


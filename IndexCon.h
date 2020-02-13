#pragma once
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include<string>
using std::string;
#include<map>
using std::map;
#include<memory>
using std::make_shared;
using std::shared_ptr;
#include <list>

//分词器
#include "Tokenizer.h"
//解析器
#include "Parser.h"
//数据库
#include "Database.h"

using std::size_t;
//应为目前文档都是顺序读取，不需要排序
struct InverseItem //倒排项
{
//文档ID 文档中的词频 在文档中的位置
    InverseItem(): docID(),tf(0),_pos() {}
    InverseItem(const uint32_t& id,const vector<uint32_t>& positions)
        :docID(id),
        tf(positions.size()),
        _pos(positions)
        {}
    InverseItem(const uint32_t& id)
        :docID(id),
        tf(1),_pos() {}     //词频最少为1,在构造的时候说明已经有一个
    InverseItem(const InverseItem &old)
        :docID(old.docID),
        tf(old.tf),
        _pos(old._pos)
         {}
    ~InverseItem()
    {
        //链表资源交给header来回收
    }

    uint32_t getDocId(){
        return docID;
    }
//private:
    const uint32_t docID;
    size_t tf;    //在该文档中的词频
    vector<uint32_t> _pos;  //在文档中的位置

};

struct InverseList //倒排列表
{
    InverseList(uint32_t i)
    : df(0),total_tf(0)
    {
       _list.push_back(InverseItem(i));
    }
    void adddf(int i){
        df = i;
    }
    void addtotaltf(){
        ++total_tf;
    }
    uint32_t getBackDocId()
    {
        auto Item = std::move(_list.back());
        return Item.getDocId();
    }
    void addItem(uint32_t docId){
        _list.push_back(InverseItem(docId));
        ++df;
    }
    void addOffset(uint32_t offset){    //因为假定是顺序读取的，所以直接吧offset加到最后的倒排项中
        auto it = _list.end();
        it->_pos.push_back(offset);
        ++it->tf;
    }
    uint32_t getDf(){
        return df;
    }
    size_t getListSize(){
        return _list.size();
    }
private:
    uint32_t df;          //出现的文档数目
    uint32_t total_tf;    //所有文档中的词频
    list<InverseItem> _list;

    //InverseListHeader& mergIntersection(const InverseListHeader&);
};
class IndexCon
{
public:
    using InverseIndex = map<uint32_t,InverseList>;
    //倒排索引 = 词元编号 + 倒排列表
    IndexCon() : iindex() { std::cout << "[ CONSTRUCT ] IndexConstructor" <<std::endl; }
    bool addDoc(BaseParser&);
    InverseList&& recall(const string&);
    //FIXME:
    ~IndexCon() {}

    static MysqlConn db;   //数据库
private:
    bool addWord2InverseList(const string& word,
                                   const string& document,
                                   uint32_t offset);
    static JiebaTokenizer tokenizer;   //分词器
    InverseIndex iindex;


};
//FIXME:为什么要写在外面?
vector<uint32_t> search_phrase(const vector<const InverseList*> &,
                               const vector<uint32_t> &);

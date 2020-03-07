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

//分词器
#include "Tokenizer.h"
//解析器
#include "Parser.h"
//数据库
#include "Database.h"

using std::size_t;
//自定义数据结构(struct)，倒排列表用链表组织：有一个HEAD的单链表
//应为目前文档都是顺序读取，不需要排序
struct InverseItem //倒排项
{
//文档ID 文档中的词频 在文档中的位置
    InverseItem(): docID(),tf(0),_pos(),next(nullptr) {}
    InverseItem(const uint32_t& id,const vector<uint32_t>& positions)
        :docID(id),
        tf(positions.size()),
        _pos(positions),
        next(nullptr)   {}
    InverseItem(const uint32_t& id)
        :docID(id),
        tf(0),_pos(),next(nullptr) {}
    InverseItem(const InverseItem &old)
        :docID(old.docID),
        tf(old.tf),
        _pos(old._pos),
        next(nullptr) {}
    ~InverseItem()
    {
        //链表资源交给header来回收
    }
    InverseItem *next;
//private:
    const uint32_t docID;
    size_t tf;    //在该文档中的词频
    vector<uint32_t> _pos;  //在文档中的位置

};

struct InverseItemHeader : InverseItem
{
    InverseItemHeader() : InverseItem(),df(0),total_tf(0) {}
    InverseItemHeader(const InverseItemHeader &old) : InverseItem(),df(old.df),total_tf(old.total_tf)
    {
        InverseItem *p2 = old.next;
        if(p2)
        {
            //复制第一个InverseItem节点
            this-> next = new InverseItem(*p2);
            
            InverseItem *p1 = this->next;
            while(p2->next)
            {
                p1->next = new InverseItem(*(p2->next));
                p2 = p2->next;
                p1 = p1->next;
            }
        }
    }
    ~InverseItemHeader()
    {
        InverseItem *p;

        while(next)
        {
            p = next->next;
            delete next;
            next = p;
        }
        next = nullptr;
    }
    uint32_t df;          //出现的文档数目
    uint32_t total_tf;    //所有文档中的词频

    //InverseItemHeader& mergIntersection(const InverseItemHeader&);
};
class IndexCon
{
public:
    using InverseIndex = map<uint32_t,InverseItemHeader *>;
    //倒排索引 = 词元编号 + 倒排列表
    IndexCon() : iindex() { std::cout << "[ CONSTRUCT ] IndexConstructor" <<std::endl; }
    bool addDoc(BaseParser&);
    shared_ptr<InverseItemHeader> recall(const string&);
    ~IndexCon() {}

    static MysqlConn db;   //数据库
private:
    bool addWord2InverseItem(const string& word,
                                   const string& document,
                                   uint32_t offset);
    static JiebaTokenizer tokenizer;   //分词器
    InverseIndex iindex;


};
//FIXME:为什么要写在外面?
vector<uint32_t> search_phrase(const vector<const InverseItem*> &,
                               const vector<uint32_t> &);

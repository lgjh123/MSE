#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <algorithm>
using std::sort;

#include "IndexCon.h"
#include "Database.h"

using namespace std;

//静态成员初始化
JiebaTokenizer IndexCon::tokenizer = JiebaTokenizer();
MysqlConn IndexCon::db = MysqlConn();
vector<uint32_t> search_phrase(const vector<const InverseItem*>& cursors, 
                      const vector<uint32_t>& offsets) {
   vector<uint32_t> phrase_positions;

   // 初始化游标
   vector<vector<uint32_t>::const_iterator> position_its(cursors.size());
   for(int i=0; i!=position_its.size(); ++i){
      position_its[i] = begin(cursors[i]->_pos);
   }

   // 开始搜索短语位置
   while(position_its[0] != cursors[0]->_pos.end()){
      const uint32_t pos{*(position_its[0])};
      uint32_t next_pos{0};

      // 移动除了第一个位置向量以外的游标
      for(int i=1; i!=position_its.size(); ++i){
         while(position_its[i] != cursors[i]->_pos.end() 
               && *(position_its[i]) - offsets[i] < pos) {
            ++(position_its[i]);
         }
         // 其中一个词找完了全部的位置
         if(position_its[i] == cursors[i]->_pos.end()) 
            return phrase_positions;

         // 更新 next_pos
         if(*(position_its[i]) - offsets[i] > pos 
            && *(position_its[i]) - offsets[i] > next_pos) {
               next_pos = *(position_its[i]) - offsets[i];
            }
      }
      if(next_pos > 0){
         // 说明第一个位置迭代器指向的位置不是候选短语
         while(position_its[0] != cursors[0] -> _pos.end()
                && *(position_its[0]) < next_pos)
            ++(position_its[0]);
      }
      else{
         // 找到了短语位置
         phrase_positions.push_back(pos);
         ++(position_its[0]);
      }
   }
   return phrase_positions;
}
bool IndexCon::addDoc(BaseParser& parser){
    string line;    //临时变量，保存line
    vector<cppjieba::Word> jiebawords;
    uint32_t document_offset{1};   //在doc中的偏移量
    while(parser.getContent(line)){   //将line的引用传入parser
        IndexCon::tokenizer._tokenizer(line,jiebawords,true);
        //使用分词器将line分词，结果存入jiebawords中
        for(auto jiebaword : jiebawords) {
            addWord2InverseList(jiebaword.word,
                                parser.getFilepath(),
                                jiebaword.offset+document_offset );
        //为分的词语构建倒排列表，添加入倒排索引
        }
        document_offset += line.size() + 1; //换行符,jiebaword只能记录一行
    }
    return true;
}
//在倒排索引中，构建传入word的倒排列表
bool IndexCon::addWord2InverseList(const string& word,
                                   const string& document,
                                   uint32_t offset){
    uint32_t wordId, documentId;
    DBConn::STATE rc;
    if((rc=db.getWordId(word,wordId))){
        db.putNewWord(word,wordId);
    }

    if((rc = db.getDocumentId(document,documentId))){
        db.putNewDoc(document,documentId);
    }

    if(iindex.find(wordId) == iindex.end()){
        //当前词不存在
        iindex[wordId] = InverseList(documentId);
        iindex[wordId].adddf(1);
    }
    //此时词一定在iindex中
    auto it = iindex.find(wordId);   //map用红黑树O(logn)  有序
    auto invList = std::move(it->second);      //可以用移动构造
    invList.addtotaltf();   //total_tf + 1;
    if(invList.getBackDocId() != documentId){
        //当前的documentId不是之前的Id 尾插1,2,3      !!!!增加倒排项的情况  FIXME:可以用find      //假定文档都是顺序读取的
        //假定docID是按照顺序递增的
        invList.addItem(documentId);
    }else{//就是在第一个docId       !!不用增加倒排项,在倒排项中加一个offset，词频++
        invList.addOffset(offset);    
    }
    return true;
}

///返回对应词的倒排列表
//查询时候的回调函数
InverseList&& IndexCon::recall(const string& query)    //query用户请求的语句
{
    vector<cppjieba::Word> queryWords,validQueryWords;
    this->tokenizer._tokenizer(query,queryWords,false);
    //TODO: this?
    //只保留索引中存在的词，这里是从数据库来查询，
    //其实通过iindex查也可以
    copy_if(queryWords.begin(),queryWords.end(),back_inserter(validQueryWords),
            [ ](const cppjieba::Word& jiebaword){
                uint32_t temp;
            return db.getWordId(jiebaword.word, temp) == 0;
            //return iindex.find(jiebaword.word)!= iindex.end();     //需要一个word和wordId的对应表,便于查找
            });   //lambda表达式
    //validQueryWords.shrink_to_fit();
    
    if(validQueryWords.size() == 0){
        return std::move(InverseList(0));   //返回对象的右值引用
    }else if (validQueryWords.size() == 1){
        uint32_t wordid;
        db.getWordId(validQueryWords[0].word, wordid);
        return std::move(InverseList(iindex.find(wordid)->second));
    }else{  //vailQueryWords.size() > 1
        //多个查询词
        //根据单词出现的文档数目排序，出现越少的单词越靠前
        sort(validQueryWords.begin(),validQueryWords.end(),
             [this](const cppjieba::Word& w1,const cppjieba::Word& w2){
                uint32_t id1,id2;
                db.getWordId(w1.word, id1);
                db.getWordId(w2.word, id2);
                return this->iindex.find(id1)->second.getDf() < this->iindex.find(id2)->second.getDf();
             });

        //初始化游标
        vector<InverseList> cursors(validQueryWords.size());
        //FIXME:validQueryWords类型
        //查找所有有效词语“倒排列表”！
        for(int i = 0; i != (int)validQueryWords.size(); ++i){
            uint32_t wordid;
            db.getWordId(validQueryWords.at(i).word, wordid); //at 返回对应元素的引用     传的都是引用
            cursors.at(i) = iindex.find(wordid)->second;
        }
        
        auto res = make_shared<InverseList>();
        while(cursors[0].getListSize() != 0){    //第一个有效词的倒排列表不为空      //FIXME:从第一个词的到排列表的第一个(倒排项de)文档开始找
                                                                                    //这个循环要遍历第一个词的每一个倒排项
            uint32_t  doc_id{cursors[0].getBackDocId()};
            uint32_t  next_doc_id{0};

            // 移动除了第一篇以外每一篇文档的游标
         for(int i=1; i!=cursors.size(); ++i){
            /*while(cursors[i] && cursors[i]->docID < doc_id)
               cursors[i] = cursors[i]->next;

            // 其中一个词找完了全部的文档
            if(!cursors[i]) {
               return res;
            }*/
            if(cursors[i].find(doc_id) != cursors[i].end());
               
            // 更新 next_doc_id
            if(cursors[i]->docID > doc_id && cursors[i]->docID > next_doc_id) {
               next_doc_id = cursors[i]->docID;
            }
         }
         if(next_doc_id>0){
            // 说明第一篇文档当前指向的文档不属于候选集合（没有出现在其它所有单词的到排列表中）
            while(cursors[0] && cursors[0]->docID < next_doc_id)
               cursors[0] = cursors[0]->next;
         }
         else{
            // 找到了一个文档，所有query单词都在里面出现过
            vector<uint32_t> offsets(validQueryWords.size());
            for (int i=0; i!=offsets.size(); ++i){
               offsets[i] = validQueryWords[i].offset;
            }
            vector<uint32_t> phrase_positions = search_phrase(cursors, offsets);
            InverseList* temp = new InverseList(doc_id);
            temp->tf = phrase_positions.size();
            temp->_pos = phrase_positions;
            temp->next = res->next;
            res->next = temp;
            res->df += 1;
            res->total_tf += temp->tf;
            // 移动游标指向下一个位置
            cursors[0] = cursors[0]->next;
        }
      }
      return res;
    }
}

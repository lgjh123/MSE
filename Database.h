#pragma once
#include <mysql/mysql.h>

#include <iostream>
using std::cout;
using std::endl;
using std::string;

//在经常变的地方使用模式
class DBConn
{
public:
    using STATE = unsigned;   //省略int
    DBConn() {
        cout  << "[ construct ] DBConn "<<endl;
    }
    //Factory函数
    STATE initial(const string& host, const string& user,
                  const string& pwd, const string& db){
        //动态绑定派生类的虚函数
        this->init();
        this->connect(host,user,pwd,db);
        return 0;
    }
//No.7为多态基类声明virtual析构函数
    virtual ~DBConn(){
        cout << "[ deconstruct ] BDConn" << endl;
    };

    virtual STATE init() = 0; //pure virtual function
    virtual STATE connect(const string&,const string&,
                          const string&,const string&) = 0;
    //pure virtual function
    
    STATE getWordId(const string&,uint32_t&); //从数据库中获得wordId
    STATE getDocumentId(const string&,uint32_t&);
    STATE getWordById(const uint32_t&,string&);
    STATE getDocumentById(const uint32_t&,string&);
    STATE putNewDoc(const string&,uint32_t&);
    STATE putNewWord(const string&,uint32_t&);
//基类提供接口，和实现的接口，子类自己完成接口的实现
protected: //如果其他子类参数和这些参数不同FIXME:
    //make sure just one row will be return
    virtual STATE _selectOneByOne(const string&,const string&,const string&,
                                  const string&,string&) = 0;
    virtual STATE _insertOne(const string&,const string&,const string&) = 0;
    virtual STATE _updateOneByOne(const string&,const string&,const string&,
                                  const string&,const string&) = 0;
    //data member
    STATE state;
    static string WORD_TABLE_NAME;
    static string DOC_TABLE_NAME;

};

class MysqlConn : public DBConn{
public:
    MysqlConn() : DBConn(){
        cout << "[ construct ]MysqlConn "<< endl;
    }
    ~MysqlConn(){
        cout <<"[ deconstruct ] MysqlConn " <<endl;
        mysql_close(&conn);
    }
    virtual STATE init () override{
        if(mysql_init(&conn)) return 0;
        else return 1;
    }
    virtual STATE connect(const string&,
                          const string&,
                          const string&,
                          const string&) override;
private:
    virtual STATE _selectOneByOne(const string&,const string&,
                                  const string&,const string&,
                                  string&) override;
    virtual STATE _insertOne(const string&, const string&,
                             const string&) override;
    virtual STATE _updateOneByOne(const string&, const string&, 
                                  const string&, const string&,
                                  const string&) override;
    void freeResult(MYSQL_RES*);
    MYSQL conn;
};

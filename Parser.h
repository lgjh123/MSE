#pragma once

#include <fstream>
using std::ifstream;

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;
using std::ios;

class BaseParser{
public:
    BaseParser() = default;
    BaseParser(const string& path)
        :ifs(path,ios::in),filepath(path) {  } //不会创建file？
    ~BaseParser(){
        if(ifs.is_open())
            ifs.close();
        ifs.clear();
    }
    virtual bool getContent(string&) = 0;
    bool open(const string&);
    const string getFilepath() const {
        return filepath;
    }
protected:   //类外普通函数不能访问，任何权限继承的子类都可以直接访问
    ifstream ifs;
    string filepath;
    FILE fp;
};
class DocParser : public BaseParser{
public:
    DocParser() : BaseParser() {}
    DocParser(const string& path): BaseParser(path) {  }
    bool getContent(string&) override;
};

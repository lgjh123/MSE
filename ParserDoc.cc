#include "Parser.h"

bool BaseParser::open(const string& path){
    if(ifs){
        if(ifs.is_open()){
            //当前文件没有读完
            return false;
        }else {
            ifs.open(path);
            filepath = path;
            return ifs.good();
        }
    }else{
        ifs.close();
        ifs.clear();
        ifs.open(path);
        return ifs.good();
    }
}

bool DocParser::getContent(string& line) {
    if(ifs){
        if(ifs.is_open()){
            std::getline(ifs,line);
            return ifs.good();
        }
        //到这里意味着open文件或者ifs已经关闭 FIXME:?
        return false;
    }else
        return false; //文件不存在
}

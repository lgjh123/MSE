#include "wikiload.cc"

int main()
{   
    const char* path = "/media/lee/LENOVO/zhwiki-20190720-pages-articles-multistream.xml";
    load_wikipedia_dump(path,100000)    ;
    return 0;

}

#include <stdio.h>
        #include <iostream>
        using std::cout;
#include <errno.h>
#include <expat.h>

#include <string.h>
#include "wikiload_callback.h"
#include <string>
using std::string;
string title;
string body;

/* Wikipedia词条XML标签中的部分 */
typedef enum {
  IN_DOCUMENT,          /* 以下几种状态以外的状态 */
  IN_PAGE,              /* 位于<page>标签中 */
  IN_PAGE_TITLE,        /* 位于<page>标签中的<title>标签中 */
  IN_PAGE_ID,           /* 位于<page>标签中的<id>标签中 */
  IN_PAGE_REVISION,     /* 位于<page>标签中的<revision>标签中 */
  IN_PAGE_REVISION_TEXT /* 位于<page>标签中的<revision>标签中的<text>标签中 */
} wikipedia_status;

/* 在Wikipedia的解析器中用到的变量 */
typedef struct {
  wikipedia_status status;    /* 正在读取词条XML标签的哪一部分 */
  //const char* title;           /* 词条标题的临时存储区 */
  //const char* body;            /* 词条正文的临时存储区 */
  int article_count;          /* 经过解析的词条总数 */
  int max_article_count;      /* 最多要解析多少个词条 */
  add_document_callback func; /* 将解析后的文档传递给该函数 */
} wikipedia_parser;

/**
 * 遇到XML的起始标签时被调用的函数
 * @param[in] user_data Wikipedia解析器的运行环境
 * @param[in] el XML标签的名字
 * @param[in] attr XML标签的属性列表
 */
static void XMLCALL
start(void *user_data, const XML_Char *el, const XML_Char *attr[])
{
  wikipedia_parser *p = (wikipedia_parser *)user_data;
  switch (p->status) {
  case IN_DOCUMENT:
    if (!strcmp(el, "page")) {
      p->status = IN_PAGE;
    }
    break;
  case IN_PAGE:
    if (!strcmp(el, "title")) {
      p->status = IN_PAGE_TITLE;
//      string title(p->title);   //创建string对象
    } else if (!strcmp(el, "id")) {
      p->status = IN_PAGE_ID;
    } else if (!strcmp(el, "revision")) {
      p->status = IN_PAGE_REVISION;
    }
    break;
  case IN_PAGE_TITLE:
  case IN_PAGE_ID:
    break;
  case IN_PAGE_REVISION:
    if (!strcmp(el, "text")) {
      p->status = IN_PAGE_REVISION_TEXT;
 //     string body(p->body);
    }
    break;
  case IN_PAGE_REVISION_TEXT:
    break;
  }
}

/**
 * 遇到XML的结束标签时被调用的函数
 * @param[in] user_data Wikipedia解析器的运行环境
 * @param[in] el XML标签的名字
 */
static void XMLCALL
end(void *user_data, const XML_Char *el)
{
  wikipedia_parser *p = (wikipedia_parser *)user_data;
  switch (p->status) {
  case IN_DOCUMENT:
    break;
  case IN_PAGE:
    if (!strcmp(el, "page")) {
      p->status = IN_DOCUMENT;
    }
    break;
  case IN_PAGE_TITLE:
    if (!strcmp(el, "title")) {
      p->status = IN_PAGE;
    }
    break;
  case IN_PAGE_ID:
    if (!strcmp(el, "id")) {
      p->status = IN_PAGE;
    }
    break;
  case IN_PAGE_REVISION:
    if (!strcmp(el, "revision")) {
      p->status = IN_PAGE;
    }
    break;
  case IN_PAGE_REVISION_TEXT:
    if (!strcmp(el, "text")) {
      p->status = IN_PAGE_REVISION;
      if (p->max_article_count < 0 ||
          p->article_count < p->max_article_count) {
        //p->func(p->title,p->body);
        //cout << title << std::endl;
        //cout << body << std::endl;
      }
      //utstring_free(p->title);
      title = "\0";
      body =  "\0";

      //用shareptr管理title和body


      //utstring_free(p->body);
     // p->title = NULL;
     // p->body = NULL;
      p->article_count++;
    }
    break;
  }
}

/**
 * 解析XML元素中的数据时被调用的函数
 * @param[in] user_data Wikipedia解析器的运行环境
 * @param[in] data 元素中的数据
 * @param[in] data_size 数据的大小
 */
static void XMLCALL
element_data(void *user_data, const XML_Char *data, int data_size)
{
  wikipedia_parser *p = (wikipedia_parser *)user_data;
  switch (p->status) {
  case IN_PAGE_TITLE:
  title = title+data;
//    printf("title = %s\n",p->title);      //插入长度为data_size 的二进制
    break;
  case IN_PAGE_REVISION_TEXT:
  body = body+data;
  //  printf("body = %s\n",p->body);        //给p->body 追加
        
    break;
  default:
    /* do nothing */
    break;
  }
}

#define LOAD_BUFFER_SIZE 0x2000

/**
 * 加载Wikipedia的副本（XML文件），并将其内容传递给指定的函数
 * @param[in] env 存储着应用程序运行环境的结构体
 * @param[in] path Wikipedia副本的路径
 * @param[in] func 接收env，词条标题，词条正文3个参数的回调函数（参看wiser.c的223行）
 * @param[in] max_article_count 最多加载多少个词条
 * @retval 0 成功
 * @retval 1 申请内存失败
 * @retval 2 打开文件失败
 * @retval 3 加载文件失败
 * @retval 4 解析XML文件失败
 */
int
load_wikipedia_dump(const char *path, /*add_document_callback func,*/ int max_article_count)
{
  FILE *fp;
  int rc = 0;
  XML_Parser xp;
  char buffer[LOAD_BUFFER_SIZE];
  wikipedia_parser wp = {
    IN_DOCUMENT,       /* 初始状态 */
  //  NULL,              /* 词条标题的临时存储区 */
  //  NULL,              /* 词条正文的临时存储区 */
    0,                 /* 初始化经过解析的词条总数 */
    max_article_count, /* 最多要解析多少个词条 */
  /*  func */              /* 将解析后的文档传递给该函数 */
  };

  if (!(xp = XML_ParserCreate("UTF-8"))) {
    printf("cannot allocate memory for parser.");
    return 1;
  }

  if (!(fp = fopen(path, "rb"))) {
    printf("cannot open wikipedia dump xml file(%s).",
                strerror(errno));
    rc = 2;
    goto exit;
  }

  XML_SetElementHandler(xp, start, end);
  XML_SetCharacterDataHandler(xp, element_data);
  XML_SetUserData(xp, (void *)&wp);

  while (1) {
    int buffer_len, done;

    buffer_len = (int)fread(buffer, 1, LOAD_BUFFER_SIZE, fp);
    if (ferror(fp)) {
      printf("wikipedia dump xml file read error.");
      rc = 3;
      goto exit;
    }
    done = feof(fp);

    if (XML_Parse(xp, buffer, buffer_len, done) == XML_STATUS_ERROR) {
      printf("wikipedia dump xml file parse error.");
      rc = 4;
      goto exit;
    }

    if (done || (max_article_count >= 0 &&
                 max_article_count <= wp.article_count)) { break; }
  }
exit:
  if (fp) {
    fclose(fp);
  }
  /*
  if (wp.title) {
    utstring_free(wp.title);
  }
  if (wp.body) {
    utstring_free(wp.body);
  }
  */
  XML_ParserFree(xp);
  return rc;
}
#include <iostream>
    int function(string& title,string& body)
    {
        std::cout << title << std::endl;

        std::cout << body << std::endl;
        return 0;
    };

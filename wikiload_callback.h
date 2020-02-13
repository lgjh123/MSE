#ifndef __WIKILOAD_H__
#define __WIKILOAD_H__

#include <string>
using std::string;
#include <functional>
typedef std::function<void(string& ,string&)> add_document_callback;
/*typedef void (*add_document_callback)(
                                      string& title,
                                      string& body);
*/
int load_wikipedia_dump(const char *path,
                        add_document_callback func, int max_article_count);

#endif /* __WIKILOAD_H__ */

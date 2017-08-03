#ifndef QAQ_CONFIG_H
#define QAQ_CONFIG_H

#include <string>

class config {
   public:
    config(std::string path);

    std::string work_method;
    std::string address;
    unsigned short port;
    int timeout;
    std::string encryption;
};

#endif  // QAQ_CONFIG_H

#ifndef QAQ_QAQ_SERVER_H
#define QAQ_QAQ_SERVER_H

#include "socket_server.h"

class qaq_server : public socket_server {
    typedef boost::asio::io_service io_service;
    typedef boost::asio::ip::tcp::socket tcp_socket;
    typedef boost::asio::ip::udp::socket udp_socket;
    typedef boost::shared_ptr<tcp_socket> tcp_socket_ptr;
    typedef boost::shared_ptr<udp_socket> udp_socket_ptr;

   public:
    qaq_server(const std::string &address, unsigned short port, int timeout);

    virtual ~qaq_server() = default;

    void run() {
        this->handle_connect();
        service->run();
    }

   protected:
    virtual reply_field on_tcp_connect(boost::shared_ptr<io_service> service,
                                       tcp_socket_ptr socket);

   private:
};

#endif  // QAQ_QAQ_SERVER_H

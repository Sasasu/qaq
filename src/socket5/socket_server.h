#ifndef QAQ_SOCKET_H
#define QAQ_SOCKET_H

#include <boost/asio.hpp>
#include <string>

class socket_server {
    typedef boost::asio::io_service io_service;
    typedef boost::asio::ip::tcp::socket tcp_socket;
    typedef boost::asio::ip::udp::socket udp_socket;
    typedef boost::shared_ptr<tcp_socket> tcp_socket_ptr;
    typedef boost::shared_ptr<udp_socket> udp_socket_ptr;

   public:
    socket_server(io_service *service, std::string address, unsigned short port,
                  int timeout);
    virtual ~socket_server() = default;

   protected:
    enum reply_field {
        success = 0x00,
        generak_error = 0x01,
        connect_not_allow = 0x02,
        network_unreachable = 0x03,
        host_unreachable = 0x04,
        connection_refused = 0x05,
        ttl_expired = 0x06,
        command_not_support = 0x07,
        address_type_not_support = 0x08
    };
    boost::shared_ptr<io_service> service;
    boost::asio::ip::tcp::acceptor tcp_acceptor;
    int timeout;
    int socket_cmd;
    std::vector<uint8_t> address;
    virtual reply_field on_tcp_connect(boost::shared_ptr<io_service> service,
                                       tcp_socket_ptr socket) = 0;
    void handle_connect();

   private:
    bool tcp_connect_handshake(tcp_socket_ptr socket);
};

#endif  // QAQ_SOCKET_H

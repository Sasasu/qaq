#include "qaq_server.h"

using namespace boost::asio;

qaq_server::qaq_server(const std::string& address, unsigned short port,
                       int timeout)
    : socket_server(new io_service(), address, port, timeout) {}

void qaq_server::handle_payload() {}

socket_server::reply_field qaq_server::on_tcp_connect(
    boost::shared_ptr<io_service> service, qaq_server::tcp_socket_ptr socket,
    socket_server::socket_cmd& cmd, address& addr) {
    return host_unreachable;
}

#include "socket_server.h"

#include <boost/array.hpp>
#include <boost/log/trivial.hpp>
#include <thread>

using namespace boost::asio::ip;
using namespace boost::asio;

socket_server::socket_server(io_service *service, std::string address,
                             unsigned short port, int timeout)
    : service(service), tcp_acceptor(*this->service), timeout(timeout) {
    ip::address server_address = ip::address::from_string(address);
    tcp::endpoint endpoint(server_address, port);
    tcp_acceptor.open(endpoint.protocol());
    tcp_acceptor.set_option(
        boost::asio::ip::tcp::acceptor::reuse_address(true));
    tcp_acceptor.bind(endpoint);
    tcp_acceptor.listen();

    BOOST_LOG_TRIVIAL(debug)
        << "Listen on " << server_address.to_string() << ":" << endpoint.port();
}

void socket_server::handle_connect() {
    BOOST_LOG_TRIVIAL(trace) << "Wait for new connect";
    boost::shared_ptr<io_service> connect_service(new io_service);
    boost::shared_ptr<tcp_socket> connect_socket(
        new tcp_socket(*connect_service));
    this->tcp_acceptor.accept(*connect_socket);
    std::thread([this, &connect_service, &connect_socket]() {
        BOOST_LOG_TRIVIAL(trace) << "New thread";
        if (tcp_connect_handshake(connect_socket)) {
            socket_server::reply_field reply =
                this->on_tcp_connect(connect_service, connect_socket);
            std::vector<uint8_t> buff = {0x05, reply, 0x00};
            buff.insert(buff.end(),this->address.begin(), this->address.end());
            connect_socket->send(buffer(buff));
            switch (reply) {
                case success:
                    break;
                case host_unreachable:
                    BOOST_LOG_TRIVIAL(trace) << "Socket close beacuse host_unreachable";
                    connect_socket->close();
                    break;
                default:
                    break;
            }
        } else {
            BOOST_LOG_TRIVIAL(trace) << "Socket close";
            connect_socket->close();
        }
        BOOST_LOG_TRIVIAL(trace) << "End thread";
    }).detach();
    return handle_connect();
}

template <int size>
inline size_t get_size() {
    return size * sizeof(uint8_t);
}

template <typename T>
inline bool handshake_step1(
    boost::shared_ptr<boost::asio::ip::tcp::socket> &socket, T buff) {
    read(*socket, buffer(buff), transfer_exactly(get_size<2>()));
    if (buff[0] != 0x05) {
        BOOST_LOG_TRIVIAL(error) << "Not support socket version";
        return false;
    }
    int8_t i = buff[1];
    read(*socket, buffer(buff), transfer_exactly(i * get_size<1>()));
    for (int8_t j = 0; j < i; ++j) {
        if (buff[j] == 0x00) {
            BOOST_LOG_TRIVIAL(info) << "Open socket5 connet";
            socket->send(buffer("\x05\x00", 2));
            return true;
        }
    }
    BOOST_LOG_TRIVIAL(error) << "Not support socket method";
    socket->send(buffer("\x05\xff", 2));
    return false;
}

template <typename T>
inline bool handshake_step2(
    boost::shared_ptr<boost::asio::ip::tcp::socket> &socket, T buff, int &cmd,
    std::vector<uint8_t> &address) {
    read(*socket, buffer(buff), transfer_exactly(get_size<3>()));
    if (buff[0] != 0x05 || buff[2] != 0x00) {
        BOOST_LOG_TRIVIAL(error) << "Not support socket version";
        return false;
    }
    cmd = buff[1];
    BOOST_LOG_TRIVIAL(trace) << "Get cmd " << std::hex << buff[1];

    read(*socket, buffer(buff), transfer_exactly(get_size<1>()));
    address.push_back(buff[0]);
    switch (buff[0]) {
        case 0x01:  // ipv4
            read(*socket, buffer(buff), transfer_exactly(get_size<4>()));
            for (int i = 0; i < 4; ++i) address.push_back(buff[i]);
            break;
        case 0x03:  // domain name
            read(*socket, buffer(buff), transfer_exactly(get_size<1>()));
            address.push_back(buff[1]);
            read(*socket, buffer(buff),
                 transfer_exactly(sizeof(uint8_t) * buff[1]));
            for (int i = 0; i < buff[1]; ++i) address.push_back(buff[i]);
            break;
        case 0x04:  // ipv6
            read(*socket, buffer(buff), transfer_exactly(get_size<16>()));
            for (int i = 0; i < 16; ++i) address.push_back(buff[i]);
            break;
        default:
            BOOST_LOG_TRIVIAL(error) << "Not support address type";
            return false;
    }
    BOOST_LOG_TRIVIAL(trace) << "Get remote address";

    read(*socket, buffer(buff), transfer_exactly(get_size<2>()));
    for (int i = 0; i < 2; ++i) address.push_back(buff[i]);
    BOOST_LOG_TRIVIAL(trace) << "Get remote port";

    return true;
}

bool socket_server::tcp_connect_handshake(
    socket_server::tcp_socket_ptr socket) {
    try {
        boost::array<uint8_t, 64> buff = {};
        if (!handshake_step1(socket, buff)) {
            return false;  // close socket by client
        }
        if (!handshake_step2(socket, buff, this->socket_cmd, this->address)) {
            BOOST_LOG_TRIVIAL(trace) << "Socket close";
            socket->close();
            return false;
        }
    } catch (boost::system::system_error &e) {
        BOOST_LOG_TRIVIAL(error) << "Error when socket handshake " << e.what();
        return false;
    }
    return true;
}

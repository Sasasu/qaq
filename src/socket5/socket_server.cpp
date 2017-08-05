#include "socket_server.h"

#include <boost/array.hpp>
#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>

using namespace boost::asio;

socket_server::socket_server(io_service *service, std::string address,
                             unsigned short port, int timeout)
    : service(service), tcp_acceptor(*this->service), timeout(timeout) {
    ip::address server_address = ip::address::from_string(address);
    ip::tcp::endpoint endpoint(server_address, port);
    tcp_acceptor.open(endpoint.protocol());
    tcp_acceptor.set_option(
        boost::asio::ip::tcp::acceptor::reuse_address(true));
    tcp_acceptor.bind(endpoint);
    tcp_acceptor.listen();
    BOOST_LOG_TRIVIAL(debug) << "Listening on " << server_address.to_string()
                             << ":" << endpoint.port();
}

void socket_server::handle_connect() {
    BOOST_LOG_TRIVIAL(trace) << "Waiting for new connect";
    boost::shared_ptr<io_service> connect_service(new io_service);
    boost::shared_ptr<tcp_socket> connect_socket(
        new tcp_socket(*connect_service));
    this->tcp_acceptor.accept(*connect_socket);
    BOOST_LOG_TRIVIAL(trace) << "Got new connect";
    boost::thread([this, connect_service, connect_socket]() {
        BOOST_LOG_TRIVIAL(trace) << "Created new thread";
        address addr;
        socket_cmd cmd;
        if (tcp_connect_handshake(connect_socket, addr, cmd)) {
            socket_server::reply_field reply = this->on_tcp_connect(
                connect_service, connect_socket, cmd, addr);
            std::vector<uint8_t> buff = {0x05, reply, 0x00};
            auto addr_data = addr.data();
            buff.insert(buff.end(), addr_data.begin(), addr_data.end());
            connect_socket->send(buffer(buff));
            switch (reply) {
                case success:
                    break;
                case host_unreachable:
                    BOOST_LOG_TRIVIAL(trace)
                        << "Socket will be closed beacuse host_unreachable";
                    connect_socket->close();
                    break;
                default:
                    break;
            }
        } else {
            connect_socket->close();
            BOOST_LOG_TRIVIAL(trace) << "Socket closed";
        }
        connect_service->run();
        BOOST_LOG_TRIVIAL(trace) << "Ended thread";
    });
    return handle_connect();
}

template <typename T>
inline bool handshake_step1(
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket, T buff) {
    read(*socket, buffer(buff), transfer_exactly(sizeof(uint16_t)));
    if (buff[0] != 0x05) {
        BOOST_LOG_TRIVIAL(error) << "Not support socket version";
        return false;
    }
    int8_t i = buff[1];
    read(*socket, buffer(buff), transfer_exactly(i * sizeof(uint8_t)));
    for (int8_t j = 0; j < i; ++j) {
        if (buff[j] == 0x00) {
            BOOST_LOG_TRIVIAL(info) << "Open socket5 connect";
            socket->send(buffer("\x05\x00", 2));
            return true;
        }
    }
    BOOST_LOG_TRIVIAL(error) << "Not support socket method";
    socket->send(buffer("\x05\xff", 2));
    return false;
}

std::string get_cmd_string(int cmd) {
    switch (cmd) {
        case 0x01:
            return "CONNECT";
        case 0x02:
            return "BIND";
        case 0x03:
            return "UDP ASSOCIATE";
        default:
            return "UNKNOWN";
    }
}

bool socket_server::tcp_connect_handshake(socket_server::tcp_socket_ptr socket,
                                          address &addr, socket_cmd &cmd) {
    try {
        boost::array<uint8_t, 64> buff = {};

        // step 1
        if (!handshake_step1(socket, buff)) {
            return false;  // close socket by client
        }

        // step 2
        read(*socket, buffer(buff), transfer_exactly(sizeof(uint8_t) * 3));
        if (buff[0] != 0x05 || buff[2] != 0x00) {
            BOOST_LOG_TRIVIAL(error) << "Not support socket version";
            return false;
        }
        cmd = static_cast<socket_cmd>(buff[1]);
        BOOST_LOG_TRIVIAL(trace) << "Get cmd " << get_cmd_string(buff[1]);
        addr.read_address(*socket);

    } catch (boost::system::system_error &e) {
        BOOST_LOG_TRIVIAL(error) << "Error when socket handshake " << e.what();
        return false;
    }
    return true;
}

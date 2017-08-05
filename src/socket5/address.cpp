#include "address.h"

#include <boost/log/trivial.hpp>

using namespace boost::asio;

template <int size>
inline size_t get_size() {
    return size * sizeof(uint8_t);
}

address::address(boost::asio::ip::tcp::socket &socket) { read_address(socket); }

boost::asio::ip::address address::get_address() {
    switch (this->type) {
        case IPV4:
            return ip::address_v4(*this->addressu.v4);
        case IPV6:
            return ip::address_v6(*this->addressu.v6);
        case DOMAIN_NAME:
        case ERROR:
            return ip::address();
    }
}

std::string address::get_domain_name() {
    if (this->type == DOMAIN_NAME) return *this->addressu.domain;
    return get_address().to_string();
}

std::vector<uint8_t> address::data() {
    std::vector<uint8_t> d;
    switch (this->type) {
        case IPV4:
            d.push_back(static_cast<uint8_t>(IPV4));
            std::copy(this->addressu.v4->begin(), this->addressu.v4->end(),
                      std::back_inserter(d));
            break;
        case IPV6:
            d.push_back(static_cast<uint8_t>(IPV6));
            std::copy(this->addressu.v6->begin(), this->addressu.v6->end(),
                      std::back_inserter(d));
            break;
        case DOMAIN_NAME:
            d.push_back(static_cast<uint8_t>(DOMAIN_NAME));
            std::copy(this->addressu.domain->begin(),
                      this->addressu.domain->end(), std::back_inserter(d));
            break;
        case ERROR:
            return d;
    }
    return d;
}

address::~address() {
    switch (type) {
        case IPV4:
            delete addressu.v4;
            break;
        case IPV6:
            delete addressu.v6;
            break;
        case DOMAIN_NAME:
            delete addressu.domain;
            break;
        case ERROR:
            nullptr;
    }
}

void address::read_address(ip::tcp::socket &socket) {
    std::array<uint8_t, 256> buff;
    read(socket, buffer(buff), transfer_exactly(get_size<1>()));
    type = static_cast<address_type>(buff[0]);
    switch (type) {
        case IPV4:
            read(socket, buffer(buff), transfer_exactly(get_size<4>()));
            addressu.v4 = new ipv4_t;
            std::copy_n(buff.begin(), 4, addressu.v4->begin());
            break;
        case IPV6:
            read(socket, buffer(buff), transfer_exactly(get_size<16>()));
            addressu.v6 = new ipv6_t;
            std::copy_n(buff.begin(), 16, addressu.v6->begin());
            break;
        case DOMAIN_NAME: {
            read(socket, buffer(buff), transfer_exactly(get_size<1>()));
            uint8_t size = buff[0];
            read(socket, buffer(buff),
                 transfer_exactly(sizeof(uint8_t) * size));
            addressu.domain =
                new domain_name_t(buff.begin(), buff.begin() + size);
            break;
        }
        default:
            type = ERROR;
            addressu.v4 = nullptr;
            BOOST_LOG_TRIVIAL(error) << "Not support address type";
    }

    read(socket, buffer(buff), transfer_exactly(sizeof(uint16_t)));
    port = buff[0] << 8 | buff[1];  // TODO Support big-end platform
    BOOST_LOG_TRIVIAL(trace)
        << "Get remote address " << get_domain_name() << ":" << port;
}

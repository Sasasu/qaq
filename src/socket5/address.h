#ifndef QAQ_ADDRESS_H
#define QAQ_ADDRESS_H

#include <boost/asio.hpp>

class address {
   public:
    address() = default;

    address(boost::asio::ip::tcp::socket &socket);

    ~address();

    void read_address(boost::asio::ip::tcp::socket &socket);

    boost::asio::ip::address get_address();

    std::string get_domain_name();

    std::vector<uint8_t> data();

    enum address_type {
        IPV4 = 0x01,
        DOMAIN_NAME = 0x03,
        IPV6 = 0x04,
        ERROR = 0x00,
    };
    typedef std::array<unsigned char, 4> ipv4_t;
    typedef std::array<unsigned char, 16> ipv6_t;
    typedef std::string domain_name_t;
    typedef uint16_t port_t;

   private:
    union address_u {
        ipv6_t *v6;
        ipv4_t *v4;
        domain_name_t *domain;
    };

    address_type type = ERROR;
    address_u addressu;
    port_t port;
};

#endif  // QAQ_ADDRESS_H

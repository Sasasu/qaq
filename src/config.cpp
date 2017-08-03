#include "config.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>

config::config(std::string path) : port(), timeout() {
    using namespace boost;
    const filesystem::path config_path(path);
    try {
        filesystem::fstream config_stream(path);
        BOOST_LOG_TRIVIAL(trace) << "Open file " << path;
        if (!filesystem::exists(path)) {
            throw filesystem::filesystem_error(
                "no such file",
                make_error_code(system::errc::no_such_file_or_directory));
        }
        std::string line;
        while (std::getline(config_stream, line)) {
            if (boost::starts_with(line, "#")) {
                continue;
            }
            unsigned long i = line.find("=");
            auto str = line.substr(0, i);
            auto data = line.substr(i + 1, line.length());
            BOOST_LOG_TRIVIAL(debug) << "Get Config " << str << "=" << data;

            if (str == "work_method")
                this->work_method = data;
            else if (str == "address")
                this->address = data;
            else if (str == "port")
                this->port = lexical_cast<unsigned short>(data);
            else if (str == "timeout")
                this->timeout = lexical_cast<int>(data);
            else if (str == "encryption")
                this->encryption = data;
            else
                throw_exception(bad_lexical_cast());
        }

    } catch (system::system_error &e) {
        BOOST_LOG_TRIVIAL(fatal) << "Start failed";
        if (e.code() ==
            make_error_code(system::errc::no_such_file_or_directory)) {
            BOOST_LOG_TRIVIAL(fatal) << "No file or directory";
        } else {
            BOOST_LOG_TRIVIAL(fatal) << "Config file error";
        }
        exit(255);
    } catch (bad_lexical_cast &e) {
        BOOST_LOG_TRIVIAL(fatal) << "Start failed";
        BOOST_LOG_TRIVIAL(fatal) << "Invalid configuration file";
        exit(254);
    }
}

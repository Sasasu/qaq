#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <map>

#include "config.h"
#include "qaq_server.h"

void show_usage(char *argv[]) {
    std::cerr << argv[0] << " -c config" << std::endl;
    exit(1);
}

int main(int argc, char *argv[]) {
    using namespace std;
    if (argc <= 1) {
        show_usage(argv);
    }
    map<string, string> args;
    for (int i = 1; i < argc; i++) {
        if (boost::starts_with(argv[i], "-c") ||
            boost::starts_with(argv[i], "--config")) {
            args["config_file"] = i + 1 < argc ? argv[++i] : nullptr;
        } else if (boost::starts_with(argv[i], "-h") ||
                   boost::starts_with(argv[i], "--help")) {
            args["show_help"] = "yes";
        }
    }
    if (args.find("show_help") != args.end()) {
        show_usage(argv);
    }
    config config_file(args["config_file"]);

    qaq_server qaq_server(config_file.address, config_file.port,
                          config_file.timeout);
    qaq_server.run();

    return 0;
}

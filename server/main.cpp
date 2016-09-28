#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "json/json.h"

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef server::message_ptr message_ptr;


void on_open(websocketpp::connection_hdl hdl) {}//succuessfull
void on_close(websocketpp::connection_hdl hdl) {}
void on_message(server *s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout<<"message :"<<msg->get_payload()<<std::endl;

    Json::Value root;
    Json::Value rsp;
    Json::FastWriter writer;
    rsp["msgid"] = "200";
    root.append(0);
    root.append("Ok");
    rsp["result"] = root;
    std::string rec = writer.write(rsp);
    ///{"msg":"200","result":[0,"Ok"]}

    try {
        s->send(hdl,rec, msg->get_opcode());
    } catch (const websocketpp::lib::error_code &e) {
        std::cout << "echo failed because: " << e << "(" << e.message() << ")" << std::endl;
    }
}

class Cserver {
private:
    server echo_server;

    Cserver(const Cserver &c);

    Cserver &operator=(const Cserver &c);

public:
    Cserver() {}

    ~Cserver(void) {}

    void init() {
        try {
            echo_server.set_access_channels(websocketpp::log::alevel::all);
            echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
            ///initialize asio
            echo_server.init_asio();

            //register our message handler
            echo_server.set_open_handler(bind(&on_open,::_1));
            echo_server.set_close_handler(bind(&on_close,::_1));
            echo_server.set_message_handler(bind(&on_message, &echo_server, ::_1, ::_2));
        } catch (websocketpp::exception const &e) {
            std::cout << e.what() << std::endl;
        } catch (...) {
            std::cout << "other exception" << std::endl;
        }
    }

    void listen() {
        try {
            //listen on port 9002
            echo_server.listen(9002);
        } catch (websocketpp::exception const &e) {
            std::cout << e.what() << std::endl;
        } catch (...) {
            std::cout << "other exception" << std::endl;
        }

    }

    void accept() {
        try {
            //start the server accept loop
            echo_server.start_accept();

            //start the asio io_service run loop
            echo_server.run();

        } catch (websocketpp::exception const &e) {
            std::cout << e.what() << std::endl;
        } catch (...) {
            std::cout << "other exception" << std::endl;
        }
    }
};


int main() {
    Cserver myserver;
    myserver.init();
    myserver.listen();
    myserver.accept();
    return 0;
}
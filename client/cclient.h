//
// Created by lizhen on 16-9-9.
//
#include <iostream>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <unistd.h>
#include <vector>
#include <string>

typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

//pull out the type of message sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

// This message handler will be invoked once for each incoming message. It
// prints the message and then sends a copy of the message back to the server.

class CClient {
public:
    CClient(const std::string &uri);
    CClient(){}
    void setUri(const std::string &uri);

    ~CClient(void);

    void init();
    bool connect();
    bool disconnct();
    void run();
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl,message_ptr msg);
    int csend(std::string msg);

public:
    client c;
    websocketpp::connection_hdl hdl;
    static log4cpp::Category &log;
private:
    std::string uri = "ws://localhost:9002";
    CClient& operator= (const CClient& c);
};
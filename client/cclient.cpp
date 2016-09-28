#include "cclient.h"
#include "json/json.h"

log4cpp::Category &CClient::log = log4cpp::Category::getInstance(std::string("sample"));

CClient::CClient(const std::string &uri) {
    this->uri = uri;
}

void CClient::on_open(websocketpp::connection_hdl hdl) {
    std::cout<<"client set the on_open"<<std::endl;
    this->hdl = hdl;

    Json::Value createRoom;
    Json::FastWriter writer;
    createRoom["msgid"] = "200";
    createRoom["roomid"] = "123";
    createRoom["userid"] = "456";
    createRoom["alias"] = "xxx";

    std::string sendMsg = writer.write(createRoom);
    ///sendMsg = {"alias":"xxx","msgid":"200","roomid":"123","userid":"456"};
    std::cout<<sendMsg<<std::endl;
    csend(sendMsg);
}
void CClient::on_close(websocketpp::connection_hdl hdl) {
    std::cout<<"client has been closed"<<std::endl;
}

void CClient::on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
    //std::cout<<"recive :"<<msg->get_payload()<<std::endl;
    std::string coo = "{\"msgid\":\"200\",\"result\":[0,\"Ok\"]}";
    if(msg->get_payload().compare(coo.c_str())){
        std::cout<<"on_message called with hdl: "<<hdl.lock().get()<<" ok!\n";
    }else{
        std::cout<<"on_message called with hdl: "<<hdl.lock().get()<<" error!\n";
    }
}

int CClient::csend(const std::string data) {
    try{
        c.send(hdl,data,websocketpp::frame::opcode::text);
    }catch (const websocketpp::lib::error_code &e){
        std::cout<<"echo failed because : "<<e
                 << "("<<e.message()<<")"<<std::endl;
    }
}

CClient::~CClient(void) {
    if(disconnct()){
        std::cout<<"disconnect done"<<std::endl;
    }else{
        std::cout<<"disconnect err"<<std::endl;
    }
}

void CClient::setUri(const std::string &uri) {
    this->uri = uri;
}

void CClient::init() {
    try {
        // Set logging to be pretty verbose (everything except message payloads)
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        c.init_asio();

        // Register our message handler
        c.set_open_handler(bind(&CClient::on_open,this,::_1));
        c.set_close_handler(bind(&CClient::on_close,this,::_1));
        c.set_message_handler(bind(&CClient::on_message,this,::_1,::_2));

    } catch (websocketpp::exception const &e) {
        std::cout << e.what() << std::endl;
    }
    log.info("cclient::init!");
}

bool CClient::connect() {
    try {
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return false;
        }

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        c.connect(con);
        return true;
    } catch (websocketpp::exception const &e) {
        std::cout << e.what() << std::endl;
    }
    log.info("connect!");
}

bool CClient::disconnct() {
    try {
        c.stop();
        return true;
    } catch (websocketpp::exception const &e) {
        std::cout << e.what() << std::endl;
    }
    log.info("disconnect!");
}

void CClient::run() {
    try {
        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        log.info("run!");
        c.run();
    } catch (websocketpp::exception const &e) {
        std::cout << e.what() << std::endl;
    }
}

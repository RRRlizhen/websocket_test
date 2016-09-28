//
// Created by lizhen on 16-9-9.
//
#include <iostream>
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <unistd.h>
#include <vector>
#include <string>
#include "cclient.h"
#include "json/json.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

bool init_log(){
    try{
        log4cpp::PropertyConfigurator::configure("./log4cpp.properties");
        return true;
    }catch (log4cpp::ConfigureFailure &f){
        std::cout<<"Configure Problem"<<f.what()<<std::endl;
        return false;
    }
}

void waitchild(int signo){
    pid_t pid;
    while((pid=waitpid(-1,NULL,0))>0){
        std::cout<<pid<<"has exit\n";
    }
    std::cout<<"main function exit\n";
}

void *test_connecting(void  *cli){
    CClient *client = static_cast<CClient*>(cli);
    client->init();
    if(!client->connect()){
        std::cout<<"connect error"<<std::endl;
    }
    client->run();
    return nullptr;
}

int main(int argc, char **argv) {
    std::vector<std::string> dy = {"...","/..",".|.","..\\",".|.","/.."};
    if(init_log()){
        std::cout<<"init_log"<<std::endl;
    }

    std::string uri = "ws://localhost:9002";
    if (argc == 2) {
        uri = argv[1];
    }

    pid_t MYPID[10];
    int fpid;
    for(int i = 0;i<10;i++){
        fpid = fork();
        if(fpid<0){
            std::cout<<"fork error\n";
        }else if(fpid==0){
            std::cout<<"child process= "<<getpid()<<std::endl;
            sleep(i);
            std::vector<pthread_t> vecp(10);
            for(int k = 0;k<10;k++){
                std::string which = "i:k= "+std::to_string(i)+" : "+std::to_string(k);
                CClient *client = new CClient();
                client->log.info(which);
                pthread_create(&vecp[k], nullptr,test_connecting,client);
                sleep(1);
            }

            for(int j = 0;j<10;j++){
                pthread_join(vecp[j], nullptr);
            }
            exit(0);
        }else{
            MYPID[i] = fpid;
        }
    }///for-i
    waitchild(0);
    exit(0);



    for(int i = 0;i<10;i++){
        if(MYPID[i]=fork()==0){
            ///child process
            std::vector<CClient*> vccc(10);
            for(int k = 0;k<10;k++){
                vccc[k] = new CClient();
                vccc[k]->init();
                if(!vccc[k]->connect()){
                    std::cout<<"connect error"<<std::endl;
                }
                vccc[k]->run();
            }///for
        }//if
    }

/*
    std::vector<pthread_t> vecp(100000);
    for(int i = 0;true;i++){
        CClient *client = new CClient();
        pthread_create(&vecp[i], nullptr,test_connecting,client);
        sleep(0.1);
        std::string whichClient = std::to_string(i);

        std::string out = dy[i%6]+"  ---CClient"+whichClient+" ] connected";
        CClient::log.info(out);
    }

    for(int i = 0;i<100000;i++){
        pthread_join(vecp[i], nullptr);
    }

    CClient::log.info("done!");
*/
    return 0;
}

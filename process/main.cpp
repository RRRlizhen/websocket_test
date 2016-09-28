#include<sys/wait.h>
#include<unistd.h>
#include <iostream>
#include <errno.h>
#include <stdlib.h>
using namespace std;

void waitchild(int signo){
    pid_t pid;
    while((pid = waitpid(-1, NULL,0))>0){
        cout<<pid<<"has exit\n";
    }
    cout<<"main function exit\n";
}

int main(){
    int status,i;
    pid_t MYPID[10];
    pid_t tpid;
    for(int i = 0;i<10;i++){
        tpid = fork();
        if(tpid<0) {
            cout << "fork error" << endl;
        }else if(tpid==0){
            cout<<"child process= "<<getpid()<<endl;
            sleep(i);
            for(int k = 0;k<10;k++){
                cout<<"i:k = "<<i<<":"<<k<<endl;
            }
            exit(0);
        }else{
            MYPID[i] = tpid;
        }
    }
    waitchild(0);
    exit(0);
}

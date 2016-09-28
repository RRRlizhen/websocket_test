#include <iostream>
#include <string>
#include <set>
using namespace std;


int p(int num){
    set<int> s;
    s.insert(1);
    for(int i = 2;i*i<=num;i++){
        if(num%i==0){
            s.insert(i);
            s.insert(num/i);
        }
    }


    for(auto i:s){
        cout<<i<<endl;
    }
    int sum = 0;
    for(set<int>::iterator sit = s.begin();sit!=s.end();sit++){
        sum += *sit;
    }
    if(sum==num){
        return 1;
    }else {
        return 0;
    }

}


int main(){
    bool done = false;
    std::string input;

    cout<<p(28);
}
#include<iostream>
#include<vector>
using namespace std;
int main(){
	vector<int*> vit(10);
	for(int i = 0;i<10;i++){
		vit[i] = new int;
	}
	cout<<vit.size()<<endl;
	return 0;
}

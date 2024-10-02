#include<bits/stdc++.h>
#include "custom_bencoder.h"

using namespace std;

int main(){
    bencoding::bencode_integer bInt=999;
    bencoding::string_subs sa=bInt.encode();
    bInt=45;
    cout<<bInt.get()<<endl;
    bencoding::bencode_integer bInt2;
    bInt2.decode(sa.str,sa.citer);
    cout<<bInt2.get()<<endl;
    cout<<typeid(sa).name()<<endl;
    return 0;
}
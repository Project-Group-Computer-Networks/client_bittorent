#include<bits/stdc++.h>
#include<fstream>
#include "custom_bencoder.h"

using namespace std;

int main(){
    ifstream file("sample.torrent", ios::binary);
    if (!file) {
        cerr << "Error opening .torrent file!" << endl;
        return 1;
    }
    string bencode((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    bencoding::string_subs sa(bencode);
    // sa.str=bencode;
    // sa.citer=sa.str.begin();
    cout<<sa.str<<endl;
    bencoding::bencode_dict torrent_dict;
    torrent_dict.decode(sa.str,sa.citer);
    string url;
    for(const auto &[key_,ptr_]:torrent_dict){
        cout<<key_.get()<<" "<<(*ptr_).get_as_str()<<endl;
        url=(*ptr_).get_as_str();
        break;
    }

    bencoding::bencode_dict file_info_dict;
    bencoding::string_subs snew(torrent_dict["info"]->encode());
    file_info_dict.decode(snew.str, snew.citer);
    bencoding::bencode_dict params;
    for(const auto &[key_,ptr_]:file_info_dict){
        cout<<key_.get()<<" "<<(*ptr_).get_as_str()<<endl;
        string key=key_.get();
        string val=(*ptr_).get_as_str();
        if(key_.get()=="pieces"){
            params["info_hash"]=std::make_unique<bencoding::bencode_string>(val);
        }
        if(key_.get()=="length"){
            params["left"]=std::make_unique<bencoding::bencode_string>(val);
        }
    }
    string peer_id="0MjRoxR3KuzXecjOVAF2";
    string port="49153";
    string uploaded="0";
    string downloaded="0";
    string compact="1";

    params["peer_id"]=std::make_unique<bencoding::bencode_string>(peer_id);
    params["port"]=std::make_unique<bencoding::bencode_string>(port);
    params["uploaded"]=std::make_unique<bencoding::bencode_string>(uploaded);
    params["downloaded"]=std::make_unique<bencoding::bencode_string>(downloaded);
    params["compact"]=std::make_unique<bencoding::bencode_string>(compact);
    
    bencoding::string_subs announce_request=params.encode();
   
    cout<<announce_request.str<<endl;
    
    


    
    
}
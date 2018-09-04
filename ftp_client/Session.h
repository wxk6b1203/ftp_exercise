
#include "header.h"

#pragma once
class Session {
   private:
    uint srcDataPort;
    uint desDataPort;
    string user;
    string pass;
    int mode;
    int defPort = 20;
    Net::SocketAddress addr;
    Net::StreamSocket ctrl_cnn;
    Net::StreamSocket data_cnn;
    // ActiveMethod<int, int, Session> dataGetter;
    int pasv();
    reply replyParser(string reply);
    int listGetterFunc(string name);

   public:
    Session(Net::SocketAddress addr);
    ~Session();

    reply PWD();
    int Rmdir(string);
    int Mkdir(string);
    void Close();
    reply Link();
    int CWD(string path);
    int List(string path);
    int Port();
    int Send(string pathname);
    int Login(string u, string p);
    int Download(string filename, string path);
};

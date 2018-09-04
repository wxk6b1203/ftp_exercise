#ifndef __CLASS_SESSION__
#include "Session.h"
#endif  // !_SESSION_

#pragma once
class Connector {
   private:
    // sessions[0]: main session
    int port;
    string addr;
    string user;
    string pass;
    bool logged;
    bool linked;
    vector<Session> sessions;
    Net::SocketAddress sa;

   public:
    Connector(string a, int port);
    ~Connector();
    void Exit();
    bool Logged();
    bool Linked();
    int List(string path);

    int Rmdir(string);

    int Mkdir(string pathname);
    int Port(int port);
    /* link to target server. */
    reply Link();

    /* change working dir */
    int CWD(string path);

    /* current dir */
    string PWD();

    int Send(string pathname);

    /* download file from server */
    int Download(string filename, string path);

    int Login(string u, string p, bool force = 0);
};

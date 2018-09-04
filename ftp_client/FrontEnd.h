#include "Connector.h"
#include "header.h"
#pragma once
class FrontEnd final {
   private:
    FrontEnd();
    ~FrontEnd();
    FrontEnd(FrontEnd const&);
    FrontEnd& operator=(FrontEnd const&) = delete;

    string pwd;
    map<string, string> helpMsg;
    vector<Connector> connections;
    map<string, std::function<int(vector<string>)> > command;
    bool linked;
    bool loggedin;
    void importCommandTable();
    void cmdRegister();

   public:
    static FrontEnd& Impl() {
        static FrontEnd f;
        return f;
    }
    void Start();
};

typedef FrontEnd f;
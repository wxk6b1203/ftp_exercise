#include "Connector.h"
#include "Session.h"
Connector::Connector(string a, int port)
    : logged(false), linked(false), sa(a, port), port(port) {
    try {
        sessions.push_back(Session(sa));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

Connector::~Connector() {}

// PORT h1,h2,h3,h4,p1,p2
int Connector::Port(int port) { return sessions[0].Port(); }

string Connector::PWD() {
    reply rp = sessions[0].PWD();
    if (rp.code > 400) {
        return "";
    }
    return rp.des;
}
int Connector::Rmdir(string pathname) { return sessions[0].Rmdir(pathname); }

int Connector::Download(string filename, string path) {
    return sessions[0].Download(filename, path);
}

int Connector::Mkdir(string pathname) { return sessions[0].Mkdir(pathname); }

reply Connector::Link() {
    if (!linked) {
        try {
            reply code = sessions[0].Link();

            if (code.code == StatusReady) {
                linked = true;
            }
            return code;
        } catch (const std::exception& e) {
            std::cerr << "Unable to connect server: " << e.what() << '\n';
        }
    }
    return reply{-1, ""};
}

void Connector::Exit() {
    for (auto&& i : sessions) {
        i.Close();
    }
}

int Connector::Send(string pathname) { return sessions[0].Send(pathname); }

int Connector::List(string path) { return sessions[0].List(path); }

int Connector::CWD(string path) { return sessions[0].CWD(path); }

int Connector::Login(string u, string p, bool force) {
    if (logged && !force) {
        return 0;
    } else {
        int s = sessions[0].Login(u, p);
        if (s == StatusLoggedIn) {
            user = u;
            pass = p;
            logged = 1;
        }
        return s;
    }

    return -1;
}

bool Connector::Linked() { return linked; }
bool Connector::Logged() { return logged; }
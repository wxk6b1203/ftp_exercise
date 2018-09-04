#include "Session.h"
#include "OnceBuffer.h"
#define CHECK(rp, s)    \
    if (rp.code != s) { \
        return rp.code; \
    }

Session::Session(Net::SocketAddress sa) : addr(sa), mode(0) {}

reply Session::replyParser(string rp) {
    int code;
    try {
        code = stoi(rp.substr(0, 3));
    } catch (const std::exception&) {
        return reply{-1, ""};
    }
    string rs = rp.substr(3);
    return reply{code, rs};
}

int Session::Login(string u, string p) {
    reply rp;
    try {
        ob buf;
        string format = "USER " + u + "\r\n";
        ctrl_cnn.sendBytes(format.c_str(), format.length());
        ctrl_cnn.receiveBytes(buf, buf.length());
        rp = replyParser(buf.read());

        if (rp.code >= 500) {
            return rp.code;
        }

        format.clear();

        format = "PASS " + p + "\r\n";

        ctrl_cnn.sendBytes(format.c_str(), format.length());
        ctrl_cnn.receiveBytes(buf, buf.length());
        rp = replyParser(buf.read());

        CHECK(rp, StatusLoggedIn);

    } catch (const std::exception& e) {
        return -1;
    }

    return rp.code;
}
int Session::Port() {
    mode = 1;
    return 1;
}
int Session::pasv() {
    ob buf;
    reply rp;
    if (mode == 0) {
        string format = "PASV\r\n";
        RegularExpression pasv("(\\d{1,3}),(\\d{1,3})\\)");
        ctrl_cnn.sendBytes(format.c_str(), format.length());
        ctrl_cnn.receiveBytes(buf, buf.length());

        rp = replyParser(buf.read());

        CHECK(rp, StatusPassiveMode);

        vector<string> filter;
        pasv.split(rp.des, filter);
        desDataPort = (stoi(filter[1]) << 8) | stoi(filter[2]);
        data_cnn.connect(Net::SocketAddress(addr.host(), desDataPort));
    } else {
        string format = "PORT ";
        srand((unsigned)time(NULL));
        defPort = rand() % (65536) + 1024;
        string addr = ctrl_cnn.peerAddress().host().toString();

        for (auto&& i : addr) {
            if (i == '.') {
                i = ',';
            }
        }
        format += addr;
        addr = std::to_string(defPort >> 8) + "," +
               std::to_string(defPort & 0xff) + "\r\n";
        format += addr;
        ctrl_cnn.sendBytes(format.c_str(), format.length() + 2);
        ctrl_cnn.receiveBytes(buf, buf.length());
        buf.read();
        Net::ServerSocket srv(defPort);
        data_cnn = srv.acceptConnection();
    }
    return rp.code;
}

reply Session::PWD() {
    string format = "PWD\r\n";
    ctrl_cnn.sendBytes(format.c_str(), format.size());
    ob buf(256);
    ctrl_cnn.receiveBytes(buf, buf.length());
    reply rp = replyParser(buf.read());

    if (rp.code != StatusPathCreated) {
        return rp;
    }

    rp.des = rp.des.substr(rp.des.find_first_of("\"") + 1,
                           rp.des.find_last_of("\"") - 2);
    return rp;
}

reply Session::Link() {
    ctrl_cnn.connect(addr, Timespan(60, 0));
    ctrl_cnn.setKeepAlive(true);
    ctrl_cnn.setReceiveTimeout(Timespan(60, 0));
    ctrl_cnn.setSendTimeout(Timespan(60, 0));

    ob buf;

    ctrl_cnn.receiveBytes(buf, buf.length());

    reply rp = replyParser(buf.read());
    if (rp.code == 220) {
        srcDataPort = ctrl_cnn.address().port() + 1;
    }

    else {
        ctrl_cnn.close();
    }

    return rp;
}

void Session::Close() {
    ctrl_cnn.sendBytes("QUIT\r\n", 6);
    ob buf;
    ctrl_cnn.receiveBytes(buf, buf.length());
    ctrl_cnn.close();
    data_cnn.close();
}

int Session::CWD(string path) {
    ob buf(192);
    reply rp;

    // rp.code = pasv();

    // CHECK(rp, StatusPassiveMode);

    string format = "CWD " + path + "\r\n";

    ctrl_cnn.sendBytes(format.c_str(), format.length());
    ctrl_cnn.receiveBytes(buf, buf.length());

    rp = replyParser(buf.read());

    CHECK(rp, 257);

    return rp.code;
}

int Session::Rmdir(string pathname) {
    ob buf;
    ctrl_cnn.sendBytes(("RMD " + pathname + "\r\n").c_str(),
                       pathname.length() + 6);
    ctrl_cnn.receiveBytes(buf, buf.length());
    return replyParser(buf.read()).code;
}

int Session::Mkdir(string pathname) {
    ob buf;
    ctrl_cnn.sendBytes(("MKD " + pathname + "\r\n").c_str(),
                       pathname.length() + 6);
    ctrl_cnn.receiveBytes(buf, buf.length());

    return replyParser(buf.read()).code;
}
int Session::listGetterFunc(string name) {
    Net::SocketAddress da(addr.host(), srcDataPort);

    // connect to target func
    data_cnn.connect(da, Timespan(60, 0));
    // ob buf(256)
    Net::SocketStream skstream(data_cnn);
    // data_cnn.receiveBytes(buf, buf.length());
    return 1;
}

int Session::Send(string pathname) {
    pasv();

    ob buf;
    ob dbuf;
    string format = "TYPE I\r\n";
    ctrl_cnn.sendBytes(format.c_str(), format.length());
    ctrl_cnn.receiveBytes(buf, buf.length());

    CHECK(replyParser(buf.read()), 200);

    ctrl_cnn.sendBytes(("STOR " + pathname + "\r\n").c_str(),
                       pathname.length() + 7);
    FILE* fp;
    fp = fopen(pathname.c_str(), "rb");
    int stmp;
    void* bufff = malloc(sizeof(unsigned char) * 1024);

    if (fp != NULL) {
        do {
            memset(bufff, 0, sizeof(unsigned char) * 1024);
            stmp = fread(bufff, sizeof(unsigned char), 1024, fp);
            data_cnn.sendBytes(bufff, stmp);
        } while (!feof(fp));
    }
    free(bufff);
    fclose(fp);
    ctrl_cnn.receiveBytes(buf, buf.length());

    CHECK(replyParser(buf.read()), 150);

    data_cnn.close();
    ctrl_cnn.receiveBytes(buf, buf.length());
    return replyParser(buf.read()).code;
}

int Session::Download(string filename, string path) {
    ob buf(256);
    ob dbuf(2048);
    reply rp;
    string format = "SIZE " + filename + "\r\n";
    ctrl_cnn.sendBytes(format.c_str(), format.length());
    ctrl_cnn.receiveBytes(buf, buf.length());

    rp = replyParser(buf.read());

    CHECK(rp, 213);

    float size = stof(rp.des);
    int acc = 0;
    while (size > 1000) {
        size /= 1024;
        acc += 1;
    }
    std::cout << "Size: " << '\n';
    switch (acc) {
        case 0:
            std::cout << size << "B" << '\n';
            break;
        case 1:
            std::cout << size << "KB" << '\n';
            break;
        case 2:
            std::cout << size << "MB" << '\n';
            break;
        case 3:
            std::cout << size << "GB" << '\n';
        default:
            std::cout << size << '\n';
            break;
    }

    pasv();

    ctrl_cnn.sendBytes("TYPE I\r\n", 8);
    ctrl_cnn.receiveBytes(buf, buf.length());

    CHECK(replyParser(buf.read()), 200);

    ctrl_cnn.sendBytes(("RETR " + filename + "\r\n").c_str(),
                       filename.length() + 7);

    ctrl_cnn.receiveBytes(buf, buf.length());

    CHECK(replyParser(buf.read()), 150);

    // Net::SocketStream ss(data_cnn);

    string target;
    if (path == "") {
        target = filename;
    } else {
        target = path;
    }

    FILE* fp;
    fp = fopen(target.c_str(), "wb");
    int stmp = 0;
    while ((stmp = data_cnn.receiveBytes(dbuf, dbuf.length())) != 0) {
        if (fp != NULL) {
            fwrite(dbuf, stmp, 1, fp);
            buf.read();
        }
    }
    fclose(fp);

    // ss.close();
    data_cnn.close();
    ctrl_cnn.receiveBytes(buf, buf.length());

    CHECK(replyParser(buf.read()), 226);

    return rp.code;
}

int Session::List(string path) {
    pasv();

    ob buf;
    ctrl_cnn.sendBytes("TYPE I\r\n", 8);
    ctrl_cnn.receiveBytes(buf, buf.length());
    buf.read();
    ctrl_cnn.sendBytes("LIST\r\n", 6);
    ctrl_cnn.receiveBytes(buf, buf.length());
    buf.read();
    Net::SocketStream ss(data_cnn);

    string st;

    while (!ss.eof()) {
        std::getline(ss, st);

        if (st.length() > 5) {
            std::cout << st << '\n';
        }
    }
    ctrl_cnn.receiveBytes(buf, buf.length());

    ss.close();
    data_cnn.close();
    return 1;
}
Session::~Session() { ctrl_cnn.close(); }
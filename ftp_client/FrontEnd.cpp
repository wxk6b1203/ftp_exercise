#include "FrontEnd.h"
#include "Connector.h"
#include "main.h"

string g_username = "";
string g_pass = "";
string g_server = "";

auto goodbye = [](int sig) {
    cout << "\nBye!" << endl;
    exit(sig);
};

FrontEnd::FrontEnd() : pwd(""), loggedin(false), linked(false) {}

FrontEnd::~FrontEnd() {}

/**
 * command function registation for every listed command.
 */
void FrontEnd::cmdRegister() {
    auto help = [&](vector<string> args) -> int {
        if (args.size() <= 1)
            for (auto& i : helpMsg) {
                cout << "  " << i.first << "\t\t" << i.second << "\n";
            }
        else
            cout << args[1] << ": "
                 << (helpMsg.count(args[1]) > 0 ? helpMsg[args[1]]
                                                : "Unknown command.")
                 << "\n";
        ;
        return 1;
    };

    auto cd = [&](vector<string> args) -> int {
        string holePath;
        if (!(linked && loggedin)) {
            std::cout << "Link and login first." << '\n';
        }
        if (args.size() == 2) {
            int stt = 0;
            if (args[1][0] == '/') {
                holePath = args[1];
            } else if (args[1] == "..") {
                holePath = pwd.substr(0, pwd.find_last_of('/'));
                if (holePath.length() == 0) {
                    holePath = "/";
                }
            } else if (args[1] == ".") {
                holePath = pwd;
            } else {
                string tmp = pwd == "/" ? "" : pwd;
                holePath = tmp + '/' + args[1];
            }
            stt = connections[connections.size() - 1].CWD(holePath);
            if (stt == 250) {
                pwd = holePath;
                std::cout << "Success." << '\n';
            }
        }
        return 1;
    };

    auto bye = [&](vector<string> args) -> int {
        for (auto&& i : connections) {
            i.Exit();
        }

        goodbye(0);
        return 1;
    };
    auto pwdF = [&](vector<string> args) -> int {
        if (!linked) {
            std::cout << "Link and login first." << '\n';
            return 0;
        }
        cout << connections[connections.size() - 1].PWD() << "\n";
        return 1;
    };
    // TODO: list dir
    auto ls = [&](vector<string> args) -> int {
        if (!loggedin || !linked) {
            std::cout << "Link and login first." << '\n';
            return 0;
        }
        if (args.size() < 2) {
            connections[connections.size() - 1].List(pwd);
        } else if (args[1][0] == '/') {
            connections[connections.size() - 1].List(args[1]);
        } else {
            connections[connections.size() - 1].List(pwd + '/' + args[1]);
        }
        return 1;
    };

    // TODO: list logged count
    auto logged = [](vector<string> args) -> int {
        std::cout << "Didn't implement." << '\n';
        return 0;
    };

    auto clear = [](vector<string> args) -> int {
        system("clear");
        return 0;
    };

    // TODO: login the server
    auto login = [&](vector<string> args) -> int {
        if (!linked) {
            std::cout << "Link first." << '\n';
            return 0;
        }
        if (connections.size() < 1) {
            std::cout << "Link to a server." << '\n';
            return 0;
        }
        string user = g_username;
        string pass = g_pass;

        if (args.size() < 3 && user == "" && pass == "") {
            while (user == "" || pass == "") {
                std::cout << "User: ";
                std::getline(cin, user);
                std::cout << "Password: ";
                std::getline(cin, pass);
                size_t blankSpace1 = user.find(" ");
                size_t blackSpace2 = pass.find(" ");

                if (blankSpace1 != string::npos ||
                    blackSpace2 != string::npos) {
                    std::cout
                        << "User name or password can not contain blank space."
                        << '\n';
                    user = "";
                    pass = "";
                }
            }
        }

        bool force = false;
        if (connections[0].Logged()) {
            string tmp = "";
            std::cout << "Logged in. Relog? (y/N)" << '\n';
            std::getline(cin, tmp);

            if (tmp.length() && (tmp[0] == 'Y' || tmp[0] == 'y')) {
                force = true;
            }
        }
        int stt = connections[0].Login(user, pass, force);
        switch (stt) {
            case StatusLoggedIn:
                std::cout << "Login success!" << '\n';
                pwd = connections[0].PWD();
                break;
            case StatusNotLoggedIn:
                std::cout << "Login error: username or password not correct."
                          << '\n';
                break;
            case -1:
                std::cerr << "Error: connect lost." << '\n';
                break;
            default:
                break;
        }
        loggedin = true;
        return 1;
    };

    auto link = [&](vector<string> args) -> int {
        string server = "";
        string portString = "";
        uint port = 0;
        if (g_server != "" && g_pass != "" && g_username != "") {
            server = g_server;
            port = 21;
        }
        if (args.size() < 2 && server == "") {
            std::cout << "Server: ";
            std::cin >> server;
            cin.get();
            while (!port) {
                std::cout << "Port (Default 21): ";

                std::getline(cin, portString);

                if (portString == "\n" || portString == "") {
                    port = 21;
                } else {
                    try {
                        port = stoi(portString);
                    } catch (const std::exception&) {
                        continue;
                    }
                }
            }
        } else if (args.size() >= 2 && server == "") {
            server = args[1];
            if (args.size() == 3) port = stoi(args[2]);
            while (!port) {
                std::cout << "Port (Default 21): ";

                std::getline(cin, portString);

                if (portString == "\n" || portString == "") {
                    port = 21;
                } else {
                    try {
                        port = stoi(portString);
                    } catch (const std::exception&) {
                        continue;
                    }
                }
            }
        }

        try {
            if (connections.size() > 0) {
                while (!connections.begin()->Linked()) {
                    connections.erase(connections.begin());
                }
            }

            connections.push_back(Connector(server, port));

        } catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
        }

        reply rp = connections[connections.size() - 1].Link();
        switch (rp.code) {
            case StatusUnableToConnectServer:
                std::cout << "Error: unable to connect FTP server.";
                break;
            case StatusDefault:
                std::cout << "Linked before.";
                break;
            case StatusReadyMinute:
                std::cout << "Target server temperory not available.";
                break;
            case StatusReady:
                std::cout << "Connect success: " << rp.des;

                break;
            default:
                break;
        }
        linked = true;
        return 1;
    };
    auto rmdir = [&](vector<string> args) -> int {
        int code = 0;
        if (args.size() == 2) {
            code = connections[connections.size() - 1].Rmdir(args[1]);
        }

        switch (code) {
            case 250:
                std::cout << "Success!" << '\n';
                break;

            default:
                std::cout << "Fail!" << '\n';
                break;
        }
        return 1;
    };
    auto mkdir = [&](vector<string> args) -> int {
        int code = 0;
        if (args.size() == 2) {
            code = connections[connections.size() - 1].Mkdir(args[1]);
        }

        switch (code) {
            case 257:
                std::cout << "Success!" << '\n';
                break;

            default:
                std::cout << "Fail!" << '\n';
                break;
        }
        return 1;
    };

    auto up = [&](vector<string> args) -> int {
        int code;
        if (args.size() == 2) {
            code = connections[connections.size() - 1].Send(args[1]);
        }

        switch (code) {
            case 226:
                std::cout << "Success!" << '\n';
                break;

            default:
                std::cerr << "Send fail. No such file or directory." << '\n';
                break;
        }
        return 1;
    };

    auto down = [&](vector<string> args) -> int {
        int code;
        if (args.size() < 2) {
            return 0;
        } else if (args.size() < 3) {
            code = connections[connections.size() - 1].Download(args[1], "");
        } else {
            if (args[2][0] == '/') {
                code = connections[connections.size() - 1].Download(
                    args[1], args[2] + '/' + args[1]);
            } else {
                code = connections[connections.size() - 1].Download(
                    args[1], "./" + args[2] + '/' + args[1]);
            }
        }
        if (code == 226) {
            std::cout << "Success!" << '\n';
        }
        return 1;
    };

    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("help", help));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("login", login));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("link", link));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("exit", bye));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("ls", ls));
    command.insert(std::pair<string, std::function<int(vector<string>)> >(
        "logged", logged));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("clear", clear));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("pwd", pwdF));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("cd", cd));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("down", down));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("up", up));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("mkdir", mkdir));
    command.insert(
        std::pair<string, std::function<int(vector<string>)> >("rmdir", rmdir));
}

void FrontEnd::importCommandTable() {
    std::ifstream cmdList("../command.tb", std::ios::in);
    if (cmdList.is_open()) {
        std::string line;
        while (std::getline(cmdList, line)) {
            if (!line.length()) continue;

            if (line[0] == '#' || line[0] == '\n') continue;

            int pos = line.find(' ');
            string cmd = line.substr(0, pos);
            line = line.substr(pos);

            // read command and command description
            while (line[0] == ' ' || line[0] == '\t') line = line.substr(1);

            string des = line;
            // insert command and des into command map
            helpMsg.insert(std::pair<string, string>(cmd, des));
        }

        cmdList.close();
    } else {
        std::cerr << "Unable to open file\n";
    }
}

void FrontEnd::Start() {
    // TODO: 注册 ctrl + C 为关闭连接

    cout << TITLE << "\n"
         << "Ver: " << VERSION << "\n";
    string cmd;
    RegularExpression cml("([/?\\w/?\\.?\\.?_?!?]+)");

    importCommandTable();
    cmdRegister();
    RegularExpression ex("ftp://(.*):(.*)@(.*)");
    if (argc == 2) {
        if (ex.match(argv[1])) {
            vector<string> vs;
            ex.split(argv[1], vs);
            g_username = vs[1];
            g_pass = vs[2];
            g_server = vs[3];
        }
    }

    while (1) {
        // 输出开头的那个奇葩的>
        std::cout << "> " + pwd + " ";

        // 获取一行命令
        std::getline(cin, cmd);

        // 判断 ctrl + D
        if (cin.eof()) {
            cout << "\nBye!\n";
            // TODO: stop(close) data connection and exit;
            exit(0);
        }

        if (cmd.length() <= 1) continue;

        // 将命令和参数分隔开
        vector<string> cmdArg;

        // 用于分割的 match 对象
        RegularExpression::Match p1;
        string stmp;

        while (cmd.length() > 0) {
            if (cmd[0] == ' ') {
                cmd.erase(0, 1);
                continue;
            }
            cml.match(cmd, 0, p1);
            cml.extract(cmd, stmp);
            cmd = cmd.substr(p1.offset + p1.length);
            cmdArg.push_back(stmp);
        }

        if (!helpMsg.count(cmdArg[0])) {
            std::cout << "  Unknown command." << '\n';
            continue;
        }
        command[cmdArg[0]](cmdArg);
    }
}
#ifndef _HEADER_
#include <Poco/ActiveMethod.h>
#include <Poco/Exception.h>
#include <Poco/FileStream.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/RegularExpression.h>
#include <Poco/Runnable.h>
#include <Poco/StreamCopier.h>
#include <Poco/Task.h>
#include <Poco/Thread.h>
#include <Poco/Timestamp.h>
#include <Poco/Util/Timer.h>
#include <Poco/Util/TimerTask.h>
#include <unistd.h>
#include <cctype>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#endif  // !_HEADER_
        //-I/usr/local/opt/openssl/include

#define TITLE "ping1008's FTP client"
#define VERSION "0.1.0"
#define AUTHOR "ping1008"

using namespace Poco;
using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::unique_ptr;
using std::vector;

#pragma once
struct reply final {
    int code;
    string des;
};

const int StatusInitiating = 100;
const int StatusRestartMarker = 110;
const int StatusReadyMinute = 120;
const int StatusAlreadyOpen = 125;
const int StatusAboutToSend = 150;

const int StatusCommandOK = 200;
const int StatusCommandNotImplemented = 202;
const int StatusSystem = 211;
const int StatusDirectory = 212;
const int StatusFile = 213;
const int StatusHelp = 214;
const int StatusName = 215;
const int StatusReady = 220;
const int StatusClosing = 221;
const int StatusDataConnectionOpen = 225;
const int StatusClosingDataConnection = 226;
const int StatusPassiveMode = 227;
const int StatusLongPassiveMode = 228;
const int StatusExtendedPassiveMode = 229;
const int StatusLoggedIn = 230;
const int StatusLoggedOut = 231;
const int StatusLogoutAck = 232;
const int StatusRequestedFileActionOK = 250;
const int StatusPathCreated = 257;

const int StatusUserOK = 331;
const int StatusLoginNeedAccount = 332;
const int StatusRequestFilePending = 350;

const int StatusNotAvailable = 421;
const int StatusCanNotOpenDataConnection = 425;
const int StatusTransfertAborted = 426;
const int StatusInvalidCredentials = 430;
const int StatusHostUnavailable = 434;
const int StatusFileActionIgnored = 450;
const int StatusActionAborted = 451;
const int Status452 = 452;

const int StatusBadCommand = 500;
const int StatusBadArguments = 501;
const int StatusNotImplemented = 502;
const int StatusBadSequence = 503;
const int StatusNotImplementedParameter = 504;
const int StatusNotLoggedIn = 530;
const int StatusStorNeedAccount = 532;
const int StatusFileUnavailable = 550;
const int StatusPageTypeUnknown = 551;
const int StatusExceededStorage = 552;
const int StatusBadFileName = 553;
const int StatusUnableToConnectServer = -1;
const int StatusDefault = 0;
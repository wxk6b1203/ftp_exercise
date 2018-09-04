package main

import (
	"net"
	"os"
	"regexp"
	"strconv"
	"time"

	"coding.net/ftp_server/internal"
	"coding.net/ftp_server/worker"
	logger "github.com/sirupsen/logrus"
)

var defPort = 21
var defAddr = ""

func main() {
	log := logger.New()
	internal.Welcome()

	if len(os.Args) == 2 {
		if m, _ := regexp.MatchString(`\d{0,3}\.\d{0,3}\.\d{0,3}\.\d{0,3}`, os.Args[1]); !m {
			log.Warn("Local IP error. Use default IP.")
		} else {
			defAddr = os.Args[1]
		}
	}
	if len(os.Args) == 3 {
		if m, _ := regexp.MatchString(`\d{0,3}\.\d{0,3}\.\d{0,3}\.\d{0,3}`, os.Args[1]); !m {
			log.Warn("Local IP error. Use default IP.")
		} else {
			defAddr = os.Args[1]
		}
		if as, err := strconv.Atoi(os.Args[2]); err == nil && as > 1 && as < 65535 {
			log.Info("Listen port is " + os.Args[2])
			defPort = as
		}
	}

	// set default pasv ip
	if defAddr == "" {
		addrs, _ := net.InterfaceAddrs()
		for _, a := range addrs {
			if ipnet, ok := a.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
				if ipnet.IP.To4() != nil {
					defAddr = ipnet.IP.String()
				}
			}
		}
	}
	service := ":" + strconv.Itoa(defPort)
	tcpAddr, err1 := net.ResolveTCPAddr("tcp", service)
	link, err2 := net.ListenTCP("tcp", tcpAddr)
	if err1 != nil || err2 != nil {
		log.Fatal(err2)
	}

	log.Info("Listening on   :" + strconv.Itoa(defPort))
	for {
		duation, _ := time.ParseDuration("201s")
		timeout := time.Now().Add(duation)
		//形成/接受一个TCP链接
		conn, _ := link.AcceptTCP()
		conn.SetDeadline(timeout)
		// 生成一个handler
		worker.NewSessionWorker(conn, defAddr).Handle()
	}
}

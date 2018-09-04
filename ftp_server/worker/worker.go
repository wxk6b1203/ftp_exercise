package worker

import (
	"net"
	"strconv"
	"strings"
	"sync"
	"time"

	"coding.net/ftp_server/internal"
	logger "github.com/sirupsen/logrus"
)

// Worker for every connection
type Worker struct {
	L sync.Mutex

	moded    bool
	linked   bool
	usered   bool
	exit     bool
	connErr  bool
	passed   bool
	root     string
	passwd   string
	username string
	pasvAddr string
	pwd      string
	ctrlCnn  *net.TCPConn
	dataCnn  *net.TCPConn

	timeout chan error
}

var (
	count    = 0
	log      = logger.New()
	callback map[string]func(args []string)
)

// NewSessionWorker create a new worker
func NewSessionWorker(conn *net.TCPConn, pasvAddr string) *Worker {
	return &Worker{
		pwd:      "",
		linked:   true,
		usered:   false,
		exit:     false,
		passed:   false,
		moded:    false,
		ctrlCnn:  conn,
		pasvAddr: pasvAddr,
	}
}

func cmdParser(raw string) []string {
	res := strings.SplitN(raw, " ", 2)
	if len(res) > 1 {
		res[1] = res[1][0 : len(res[1])-2]
	} else {
		res[0] = res[0][0 : len(res[0])-2]
	}
	return res
}

// Handle handle FTP session
func (w *Worker) Handle() {
	go w.handler()
	go w.timeoutHandler()
	count++
	log.WithFields(logger.Fields{
		"src": w.ctrlCnn.RemoteAddr().String(),
		"cnt": count,
	}).Info("Connected.")

}

func (w *Worker) timeoutHandler() {
	select {
	case <-w.timeout:
		w.L.Lock()
		w.ctrlCnn.CloseRead()
		w.connErr = true
		w.L.Unlock()
		return
	}
}

func (w *Worker) checkErr(err error) {
	if err != nil {
		w.timeout <- err
	}
}

func (w *Worker) handler() {
	w.linked = true
	timeouterr := make(chan error)
	_, err := w.ctrlCnn.Write([]byte("220 " + internal.Banner + "\r\n"))
	if err != nil {
		timeouterr <- err
		return
	}
	setTimeOut := time.Tick(200 * time.Second)

	go func() {
		w.L.Lock()
		defer w.L.Unlock()

		<-setTimeOut

		if w.linked == true {
			w.ctrlCnn.Write([]byte("421 Timeout.\r\n"))
			count--
			log.WithFields(logger.Fields{
				"cnt":  strconv.Itoa(count),
				"src":  w.ctrlCnn.RemoteAddr().String(),
				"user": w.username,
			}).Info("Timeout.")
			w.ctrlCnn.Close()
			w.linked = false
		}

	}()

	if w.connErr {
		return
	}

	for {
		if w.connErr || w.exit {
			w.ctrlCnn.Close()
			return
		}
		buf := make([]byte, 128)
		cnt, err := w.ctrlCnn.Read(buf) //读取请求
		w.checkErr(err)
		parsedCmd := cmdParser(string(buf[:cnt])) //解析命令
		switch parsedCmd[0] {                     //巨大的switch来调用特定函数
		case "USER":
			w.user(parsedCmd)
		case "PASS":
			w.pass(parsedCmd)
		case "PORT":
			w.port(parsedCmd)
		case "SYST":
			w.syst(parsedCmd)
		case "PWD":
			w.pwds(parsedCmd)
		case "CWD":
			w.cwd(parsedCmd)
		case "TYPE":
			w.types(parsedCmd)
		case "PASV":
			w.pasv(parsedCmd)
		case "LIST":
			w.lists(parsedCmd)
		case "RETR":
			w.down(parsedCmd)
		case "STOR":
			w.upload(parsedCmd)
		case "QUIT":
			w.bye()
		case "SIZE":
			w.sized(parsedCmd)
		case "MKD":
			w.mkd(parsedCmd)
		case "RMD":
			w.rmd(parsedCmd)
		default:
			w.sts502()
		}
	}
}

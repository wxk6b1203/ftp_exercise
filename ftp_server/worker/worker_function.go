package worker

import (
	"bufio"
	"io"
	"io/ioutil"
	"net"
	"os"
	"os/exec"
	"path"
	"regexp"
	"strconv"
	"strings"

	logger "github.com/sirupsen/logrus"
)

//parent working directory
func (w *Worker) pwds(args []string) {
	if w.linked && w.usered && w.passed {
		_, err := w.ctrlCnn.Write([]byte(`257 ` + `"` + w.pwd + `"` + ` is the current directory` + "\r\n"))
		w.checkErr(err)
	} else if !(w.usered && w.passed) {
		w.sts530()
	} else {
		w.sts500()
	}
}

func (w *Worker) lists(args []string) {
	if w.linked && w.usered && w.passed {
		if !w.moded {
			w.sts425()
			return
		}
		w.ctrlCnn.Write([]byte("150 Here comes the directory listing.\r\n"))
		var err error
		dir := w.pwd
		if len(args) == 2 {
			if path.IsAbs(args[1]) {
				dir = args[1]
			} else {
				dir = path.Join(w.pwd, args[1])
			}
		} else {
			dir = w.pwd
		}
		// block io
		for w.dataCnn == nil {
		}

		cmd := exec.Command("ls", "-l", dir)
		stdout, err := cmd.StdoutPipe()
		cmd.Start()
		reader := bufio.NewReader(stdout)

		//实时循环读取输出流中的一行内容
		for {
			line, err2 := reader.ReadString('\n')
			if err2 != nil || io.EOF == err2 {
				break
			}
			line = strings.Replace(line, "\n", "\r\n", 1)
			w.dataCnn.Write([]byte(line))
		}

		// block until command finish
		cmd.Wait()

		if err != nil {
			w.sts500()
			return
		}
		w.dataCnn.Close()
		w.moded = false
		w.dataCnn = nil
		w.ctrlCnn.Write([]byte("226 Directory send OK.\r\n"))
	} else if w.linked {
		w.sts530()
	} else {
		w.sts500()
	}
}

func (w *Worker) mkd(args []string) {
	if w.linked && w.usered && w.passed && len(args) == 2 {
		var s string
		if args[1][0] == '/' {
			s = args[1]
		} else {
			s = path.Join(w.pwd, args[1])
		}
		os.Mkdir(s, 0666)
	} else {
		w.sts500()
	}
}

func (w *Worker) rmd(args []string) {
	if w.linked && w.usered && w.passed && len(args) == 2 {
		var s string
		if args[1][0] == '/' {
			s = args[1]
		} else {
			s = path.Join(w.pwd, args[1])
		}
		err := os.Remove(s)
		if err != nil {
			w.sts500()
		}
	} else {
		w.sts500()
	}
}

// types switching
func (w *Worker) types(args []string) {
	l := len(args) == 2
	m := w.linked && w.usered && w.passed
	if l && m {
		if args[1] == "A" {
			_, err := w.ctrlCnn.Write([]byte("200 Switching to ASCII mode.\r\n"))
			w.checkErr(err)
		} else if args[1] == "I" {
			_, err := w.ctrlCnn.Write([]byte("200 Switching to BINARY mode.\r\n"))
			w.checkErr(err)
		}
	} else if l {
		w.sts530()
	} else {
		w.sts500()
	}
}

func (w *Worker) user(args []string) {
	l := len(args) == 2
	m := w.linked
	if !(l && m) {
		w.sts530()
		return
	}
	for _, i := range users {
		if i.username == args[1] {
			_, err := w.ctrlCnn.Write([]byte("331 Please specify the password.\r\n"))
			w.checkErr(err)
			w.usered = true
			w.username = args[1]
			w.root = i.root
			w.passwd = i.pass
			return
		}
	}
	w.sts530()
}

// enter a dir
func (w *Worker) cwd(args []string) {
	if w.linked && w.usered && w.passed && len(args) == 2 {
		// 区分绝对地址和相对地址
		var dir string
		if args[1][0] == '/' {
			// Absolute directory
			dir = args[1]
		} else {
			// relevent.
			dir = path.Join(w.pwd, args[1])
		}
		file, err := os.Stat(dir)
		if err != nil || !file.IsDir() {
			if os.IsNotExist(err) {
				log.WithFields(logger.Fields{
					"src":  w.ctrlCnn.RemoteAddr().String(),
					"cnt":  strconv.Itoa(count),
					"user": w.username,
				}).Warn("Dir " + w.root + w.pwd + " not exists.")
			}
			_, erro := w.ctrlCnn.Write([]byte("550 Failed to change directory.\r\n"))
			w.checkErr(erro)
			return
		}
		if args[1][0] == '/' {
			w.pwd = args[1]
		} else {
			w.pwd = path.Join(w.pwd, args[1])
		}

		_, erro := w.ctrlCnn.Write([]byte("250 Directory successfully changed.\r\n"))
		w.checkErr(erro)
	} else if !(w.usered && w.passed) {
		w.sts530()
	} else {
		w.sts500()
	}
}

func (w *Worker) pass(args []string) {
	err := error(nil)
	l := len(args) == 2
	m := w.linked && w.usered
	if l && m {
		if w.passwd != args[1] {
			w.sts530()
			return
		}
		_, err = w.ctrlCnn.Write([]byte("230 Login Successful.\r\n"))
		log.WithFields(logger.Fields{
			"src":  w.ctrlCnn.RemoteAddr().String(),
			"cnt":  strconv.Itoa(count),
			"user": w.username,
		}).Info("Logged in.")
		w.checkErr(err)
		w.passed = true
		w.pwd = w.root
	} else {
		w.sts530()
	}
}

func (w *Worker) sts530() {
	_, err := w.ctrlCnn.Write([]byte("530 Please specify USER and PASS.\r\n"))
	w.checkErr(err)
}

func (w *Worker) sts500() {
	_, err := w.ctrlCnn.Write([]byte("500 Illegal command.\r\n"))
	w.checkErr(err)
}

func (w *Worker) port(args []string) {
	if w.linked && w.usered && w.passed && len(args) == 2 {
		reg := regexp.MustCompile(",")
		res := reg.Split(args[1], -1)
		if len(res) != 6 {
			_, err := w.ctrlCnn.Write([]byte("500 Illegal command.\r\n"))
			w.checkErr(err)
			return
		}
		portH, _ := strconv.Atoi(res[4])
		portL, _ := strconv.Atoi(res[5])
		dsrc := w.ctrlCnn.RemoteAddr().String()
		dport := (portH << 8) | portL
		for i := len(dsrc) - 1; i > 0; i-- {
			if dsrc[i] == ':' {
				dsrc = dsrc[0:i]
				break
			}
		}
		service := dsrc + ":" + strconv.Itoa(dport)
		tcpAddr, errn := net.ResolveTCPAddr("tcp", service)
		if errn != nil {
			w.sts500()
			return
		}
		w.ctrlCnn.Write([]byte("200 PORT command successful. Consider using PASV.\r\n"))
		w.dataCnn, errn = net.DialTCP("tcp", nil, tcpAddr)
		w.moded = true
		w.checkErr(errn)
	} else {
		w.sts530()
	}
}

func (w *Worker) syst(args []string) {
	if len(args) == 1 && w.linked && w.usered && w.passed {
		w.ctrlCnn.Write([]byte("215 UNIX Type: L8\r\n"))
	} else if !(w.usered && w.passed) {
		w.sts530()
	} else {
		w.sts501()
	}
}

func (w *Worker) pasv(args []string) {
	var err1 error
	var port uint16
	var tcpAddr *net.TCPAddr
	var link *net.TCPListener
	pasvaddr := w.pasvAddr
	reg := regexp.MustCompile("\\.")
	if pasvaddr == "" {
		pasvaddr = strings.Split(w.ctrlCnn.LocalAddr().String(), ":")[0]
	}
	res := reg.Split(w.pasvAddr, -1)
	str := res[0] + "," + res[1] + "," + res[2] + "," + res[3]
	for {
		port = random()
		service := ":" + strconv.Itoa(int(port))
		tcpAddr, _ = net.ResolveTCPAddr("tcp", service)
		link, err1 = net.ListenTCP("tcp", tcpAddr)
		if err1 == nil {
			break
		}
	}
	p1, p2 := strconv.Itoa(int(port>>8)), strconv.Itoa(int(port&0xff))
	w.ctrlCnn.Write([]byte("227 Entering Passive Mode " + "(" + str + "," + p1 + "," + p2 + ").\r\n"))
	w.moded = true
	go func() {
		w.dataCnn, _ = link.AcceptTCP()
		w.moded = true
		log.WithFields(logger.Fields{
			"src": w.ctrlCnn.RemoteAddr().String(),
		}).Info("227 Entering Passive Mode " + "(" + str + "," + p1 + "," + p2 + ").")
	}()
}

func (w *Worker) bye() {
	w.ctrlCnn.Write([]byte("221 Goodbye.\r\n"))
	if w.ctrlCnn != nil {
		w.ctrlCnn.Close()
	}
	if w.dataCnn != nil {
		w.dataCnn.Close()
	}
	w.exit = true
	w.linked = false
	count--
	log.WithFields(logger.Fields{
		"src":  w.ctrlCnn.RemoteAddr().String(),
		"cnt":  strconv.Itoa(count),
		"user": w.username}).Info("Logged out.")
}

func (w *Worker) sized(args []string) {
	if w.linked && w.usered && w.passed && len(args) == 2 {
		file, err := os.Stat(path.Join(w.pwd, args[1]))
		if err != nil {
			w.sts500()
		} else {
			s := strconv.FormatInt(file.Size(), 10)
			w.ctrlCnn.Write([]byte("213 " + s + "\r\n"))
			return
		}
	} else {
		w.sts500()
	}

}

func (w *Worker) down(args []string) {
	if w.linked && w.usered && w.passed && w.moded {
		dir := w.pwd
		if path.IsAbs(args[1]) {
			dir = args[1]
		} else {
			dir = path.Join(w.pwd, args[1])
		}
		file, err := os.Open(dir)
		if err != nil {
			if os.IsNotExist(err) {
				w.ctrlCnn.Write([]byte("550 File not exists.\r\n"))
				return
			}
			w.ctrlCnn.Write([]byte("550 Read error.\r\n"))
			return
		}
		bytes, err := ioutil.ReadAll(file)
		if err != nil {
			log.Warn(err)
			w.ctrlCnn.Write([]byte("550 Read error.\r\n"))
			w.moded = false
			w.dataCnn.Close()
			return
		}
		w.ctrlCnn.Write([]byte("150 Opening BINARY mode data connection for " +
			file.Name() + " (" + strconv.Itoa(len(bytes)) + " bytes)" + ".\r\n"))
		for w.dataCnn == nil {
		}
		w.dataCnn.Write(bytes)
		w.dataCnn.Close()
		w.ctrlCnn.Write([]byte("226 Transfer complete.\r\n"))
		w.moded = false
		w.dataCnn = nil
	} else if !(w.passed && w.usered) {
		w.sts530()
	} else if !w.moded {
		w.sts425()
	}
}

// upload files
func (w *Worker) upload(args []string) {
	dir := w.pwd
	if path.IsAbs(args[1]) {
		dir = args[1]
	} else {
		dir = path.Join(w.pwd, args[1])
	}
	if w.linked && w.usered && w.passed && w.moded {
		for w.dataCnn == nil {
		}
		w.ctrlCnn.Write([]byte("150 Ok to send data.\r\n"))
		file, err := os.OpenFile(dir, os.O_WRONLY|os.O_TRUNC|os.O_CREATE, 0666)
		if err != nil {
			log.Warn("Write error.")
		}
		buf := make([]byte, 4096)
		for {
			n, err := w.dataCnn.Read(buf)
			if err == io.EOF {
				break
			}
			if err != nil {
				if err != io.EOF {
					log.Warn(err.Error())
				}
			}
			buf = buf[0:n]
			file.Write(buf)
		}
		w.dataCnn.Close()
		w.moded = false
		file.Close()
		w.ctrlCnn.Write([]byte("226 File send ok.\r\n"))
	} else if !(w.usered && w.passed) {
		w.sts530()
	} else {
		w.sts500()
	}
}

func (w *Worker) sts425() {
	w.ctrlCnn.Write([]byte("425 Use PORT or PASV first.\r\n"))
}

func (w *Worker) sts501() {
	_, err := w.ctrlCnn.Write([]byte("501 Bad Arguments.\r\n"))
	w.checkErr(err)
}

//502 not implemented
func (w *Worker) sts502() {
	_, err := w.ctrlCnn.Write([]byte("502 Command not implemented.\r\n"))
	w.checkErr(err)
}

package worker

import (
	"encoding/json"
	"os"

	"github.com/sirupsen/logrus"
)

type user struct {
	username string
	root     string
	pass     string
}

var users []user

func init() {
	file, err := os.Open("userlist.json")
	if err != nil {
		if os.IsNotExist(err) {
			logrus.Fatal("User list not found.")
		}
	}
	defer file.Close()
	list := make(map[string][][]string, 0)
	if err := json.NewDecoder(file).Decode(&list); err != nil {
		log.Fatal(err)
	}
	for _, item := range list {
		for _, it := range item {
			users = append(users, user{it[0], it[1], it[2]})
		}
	}
}

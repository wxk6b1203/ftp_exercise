package internal

import (
	"fmt"
)

//Welcome print welcome msg
func Welcome() {
	fmt.Println(`Golang built FTP Server`)
	fmt.Println("Author:\t", author)
	fmt.Println("Ver:\t", version)
}

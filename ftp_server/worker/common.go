package worker

import (
	"math/rand"
	"time"

	"github.com/seehuhn/mt19937"
)

func random() uint16 {
	rng := rand.New(mt19937.New())
	rng.Seed(time.Now().UnixNano())
	for {
		s := (rng.Int31() & 0x00ffff00) >> 8
		if s > 1024 && s < 65536 {
			return uint16(s)
		}
	}
}

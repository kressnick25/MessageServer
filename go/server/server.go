package main

import "server/lib"

func main() {
	c := lib.NewChannel(1)

	println("Hello server")
}

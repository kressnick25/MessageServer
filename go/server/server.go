package main

import (
	"fmt"
	"server/lib"
)

func main() {

	store := lib.NewStore()

	a := lib.User{Id: "a"}
	b := lib.User{Id: "b"}

	err := store.Subscribe(1, &a)
	if err != nil {
		// expect
		fmt.Printf("expected: %s\n", err.Error())
	}

	store.AddNewChannel(1)

	err = store.Subscribe(1, &a)
	assert(err == nil, err)
	err = store.Subscribe(1, &b)
	assert(err == nil, err)

	err = store.SendMessage(2, lib.NewMessage(a.Id, "Test"))
	if err != nil {
		// expect
		fmt.Printf("expected: %s\n", err.Error())
	}

	err = store.SendMessage(1, lib.NewMessage(a.Id, "Message 1"))
	assert(err == nil, err)

	msgs, err := store.ReadNewMessages(1, &b)
	assert(err == nil, err)
	printMsgs(msgs)

	err = store.SendMessage(1, lib.NewMessage(a.Id, "Message 2"))
	assert(err == nil, err)
	err = store.SendMessage(1, lib.NewMessage(a.Id, "Message 3"))
	assert(err == nil, err)

	msgs, err = store.ReadNewMessages(1, &b)
	assert(err == nil, err)

	printMsgs(msgs)

	msgs, err = store.ReadNewMessages(1, &b)
	assert(err == nil, err)
	// should be empty
	printMsgs(msgs)
}

func assert(test bool, err error) {
	if !test {
		fmt.Printf("FATAL: %s\n", err.Error())
		panic(1)
	}
}

func printMsgs(msgs []*lib.Message) {
	for _, msg := range msgs {
		println(msg.Content)
	}
}

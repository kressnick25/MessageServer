package lib

import (
	"testing"
)

func TestSubsribeNoChannel(t *testing.T) {
	store := NewStore()

	a := User{Id: "a"}
	err := store.Subscribe(1, &a)
	if err == nil {
		t.Errorf("Expected err on subscribe to missing channel")
	}
}

func TestSendMessageNoChannel(t *testing.T) {
	store := NewStore()

	a := User{Id: "a"}

	err := store.SendMessage(1, NewMessage(a.Id, "Test"))
	if err == nil {
		t.Errorf("Expected err on send message to missing channel")
	}
}

func TestNoMessagesFromUnsubscribedChannel(t *testing.T) {
	store := NewStore()
	a := User{Id: "a"}
	store.AddNewChannel(1)

	err := store.SendMessage(1, NewMessage(a.Id, "Hello 1"))
	check(err, t)

	msgs, err := store.ReadNewMessages(1, &a)
	check(err, t)
	if len(msgs) != 0 {
		t.Errorf("expected 0 messages, actual %d", len(msgs))
	}
}

func TestSingleUserHappyPath(t *testing.T) {
	store := NewStore()
	a := User{Id: "a"}
	err := store.AddNewChannel(1)
	check(err, t)
	err = store.Subscribe(1, &a)
	check(err, t)

	err = store.SendMessage(1, NewMessage(a.Id, "Hello 1"))
	check(err, t)

	msgs, err := store.ReadNewMessages(1, &a)
	check(err, t)

	if len(msgs) != 1 {
		t.Errorf("expected 1 message, actual %d", len(msgs))
	}
	if msgs[0].Content != "Hello 1" {
		t.Errorf("expected message '%s', actual: '%s'", "Hello 1", msgs[0].Content)
	}

	b := User{Id: "b"}
	store.Subscribe(1, &b)

	msgs, err = store.ReadNewMessages(1, &b)
	check(err, t)

	if len(msgs) != 1 {
		t.Errorf("expected 1 message, actual %d", len(msgs))
	}
	if msgs[0].Content != "Hello 1" {
		t.Errorf("expected message '%s', actual: '%s'", "Hello 1", msgs[0].Content)
	}


	msgs, err = store.ReadNewMessages(1, &a)
	check(err, t)
	if len(msgs) != 0 {
		t.Errorf("expected 0 messages, actual %d", len(msgs))
	}


}

func check(err error, t *testing.T) {
	if err != nil {
		t.Error(err)
	}
}
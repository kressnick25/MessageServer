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

func TestUnsubscribedChannelReturnsError(t *testing.T) {
	store := NewStore()
	a := User{Id: "a"}
	store.AddNewChannel(1)

	err := store.SendMessage(1, NewMessage(a.Id, "Hello 1"))
	check(err, t)

	_, err = store.ReadNewMessages(1, &a)
	if err == nil {
		t.Errorf("Expected err on reading from unsubbed channel")
	}
}

func TestSingleUserHappyPath(t *testing.T) {
	store := NewStore()
	a := User{Id: "a"}
	err := store.AddNewChannel(1)
	check(err, t)

	err = store.Subscribe(1, &a)
	check(err, t)

	// A sends and reads own message
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

	// B Subscribes and reads A's message
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

	// A re-reading, no new messages
	msgs, err = store.ReadNewMessages(1, &a)
	check(err, t)
	if len(msgs) != 0 {
		t.Errorf("expected 0 messages, actual %d", len(msgs))
	}

	// Different senders
	err = store.SendMessage(1, NewMessage(a.Id, "Hello 2"))
	check(err, t)
	err = store.SendMessage(1, NewMessage(b.Id, "Hello 3"))
	check(err, t)

	msgs, err = store.ReadNewMessages(1, &a)
	check(err, t)
	if len(msgs) != 2 {
		t.Errorf("Expected new messages, actual: %d", len(msgs))
	}
	if msgs[0].Content != "Hello 2" || msgs[0].UserId != a.Id {
		t.Errorf("Incorrect first message. %+v", msgs[0])
	}
	if msgs[1].Content != "Hello 3" || msgs[1].UserId != b.Id {
		t.Errorf("Incorrect first message. %+v", msgs[1])
	}


	msgs, err = store.ReadNewMessages(1, &b)
	check(err, t)
	if len(msgs) != 2 {
		t.Errorf("Expected new messages, actual: %d", len(msgs))
	}
	if msgs[0].Content != "Hello 2" || msgs[0].UserId != a.Id {
		t.Errorf("Incorrect first message. %+v", msgs[0])
	}
	if msgs[1].Content != "Hello 3" || msgs[1].UserId != b.Id {
		t.Errorf("Incorrect first message. %+v", msgs[1])
	}

	// A unsubs
	store.Unsubscribe(1, &a)
	_, err = store.ReadNewMessages(1, &a)
	if err == nil {
		t.Errorf("Expected err on reading from unsubbed channel")
	}

	// B unsub all
	store.UnsubscribeAll(&b)
	_, err = store.ReadNewMessages(1, &a)
	if err == nil {
		t.Errorf("Expected err on reading from unsubbed channel")
	}

}

func check(err error, t *testing.T) {
	if err != nil {
		t.Error(err)
	}
}
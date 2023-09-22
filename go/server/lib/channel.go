package lib

import (
	"fmt"
	"hash/maphash"
	"sync"
	"time"
)

// Pub structs

type Channel struct {
	Id       int
	users    []channelUser
	messages []*Message
	subMu    sync.Mutex
	msgMu	 sync.RWMutex
}

type Message struct {
	UserId    string
	Content   string
	Timestamp time.Time
}

func NewMessage(userId string, content string) *Message {
	return &Message{userId, content, time.Now()}
}

func (m Message) Hash() uint64 {
	seed := maphash.MakeSeed()
	userHash := maphash.String(seed, m.UserId)
	contentHash := maphash.String(seed, m.UserId)
	timeHash := maphash.String(seed, m.Timestamp.String())

	// FIXME may need some improvement
	return 31 * userHash * contentHash * timeHash
}

type User struct {
	Id string
}

type channelUser struct {
	user          *User
	lastReadIndex int
}

func NewChannel(id int) *Channel {
	return &Channel{id, []channelUser{}, []*Message{}, sync.Mutex{}, sync.RWMutex{}}
}

// Subscribe a User to the Channel, if not already subscribed
func (c *Channel) Subscribe(user *User) {
	// lock here to prevent double insert, or overwriting previous insert
	// maybe change users to a Set?
	c.subMu.Lock()
	defer c.subMu.Unlock()

	if c.getChannelUser(user.Id) != nil {
		return
	}

	channelUser := channelUser{user, -1}
	c.users = append(c.users, channelUser)
}

// Unsubscribe a User from the Channel
func (c *Channel) Unsubscribe(userId string) {
	c.subMu.Lock()
	defer c.subMu.Unlock()

	for i, cUser := range c.users {
		if cUser.user.Id == userId {
			c.users = removeFromSlice(c.users, i) // FIXME do inplace, without copying
		}
	}
}

// Send a Message to the Channel
func (c *Channel) Send(message *Message) {
	c.msgMu.Lock()
	defer c.msgMu.Unlock()

	c.messages = append(c.messages, message)
}

// Get all unread Messages on the channel for a User
// returns empty slice if there are no new unread Messages
// returns error if the User is not subscribed to the channel
func (c *Channel) GetNewMessages(userId string) ([]*Message, error) {

	// TODO instead of storing the int, which can cause duplicate messages to be sent without read locking,
	// store the hash of the last message received, then send all since that message.
	c.msgMu.RLock()
	defer c.msgMu.RUnlock()

	// hmm although that would then incur a scan on every message request.
	// maybe not an issue once move to streaming? To investigate


	cUser := c.getChannelUser(userId)
	if cUser == nil {
		return nil, fmt.Errorf("User %s not subsribed to channel %d", userId, c.Id)
	}
	lastIdx := len(c.messages) - 1
	newMessages := c.messages[cUser.lastReadIndex + 1:]
	cUser.lastReadIndex = lastIdx
	return newMessages, nil
}

func (c *Channel) GetAllMessages(userId string) []*Message {
	c.msgMu.RLock()
	defer c.msgMu.RUnlock()

	cUser := c.getChannelUser(userId)
	if cUser == nil {
		return nil
	}

	newMessages := c.messages
	cUser.lastReadIndex = len(c.messages) - 1
	return newMessages
}

func (c *Channel) getChannelUser(userId string) *channelUser {
	for i, cUser := range c.users {
		if cUser.user.Id == userId {
			return &c.users[i]
		}
	}
	return nil
}

// remove a element from a slice and return the new slice
func removeFromSlice(s []channelUser, i int) []channelUser {
	s[i] = s[len(s)-1]
	return s[:len(s)-1]
}

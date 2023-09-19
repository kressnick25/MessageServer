package channel

import "time"

// Pub structs

type Channel struct {
	Id int
	users []channelUser
	messages []Message
}

type Message struct {
	User *User
	Content string
	Timestamp time.Time
}

type User struct {
	Id string
}

type channelUser struct {
	user *User
	lastReadIndex int
}

// Subscribe a User to the Channel
func (c *Channel) Subscribe(user *User) {
	channelUser := channelUser {user, 0}
	c.users = append(c.users, *&channelUser)
}

func NewChannel(id int) *Channel {
	return &Channel{id, []channelUser{}, []Message{}}
}

// Unsubscribe a User from the Channel
func (c *Channel) Unsubscribe(userId string) {
	for i, cUser := range c.users {
		if cUser.user.Id == userId {
			c.users = removeFromSlice(c.users, i) // FIXME do inplace, without copying	
		}
	}
}

// Send a Message to the Channel
func (c *Channel) Send(message Message) {
	c.messages = append(c.messages, message)
}

// Get all unread Messages on the channel for a User
// returns empty slice if there are no new unread Messages
// returns nil if the User is not subscribed to the channel
func (c *Channel) GetNewMessages(userId string) []Message {
	cUser := c.getChannelUser(userId)
	if cUser == nil {
		return nil
	}
	lastIdx := len(c.messages) - 1
	if cUser.lastReadIndex == lastIdx {
		return []Message{}
	}
	newMessages := c.messages[cUser.lastReadIndex:lastIdx]
	cUser.lastReadIndex = lastIdx
	return newMessages
}

func (c *Channel) getChannelUser(userId string) *channelUser {
	for _, cUser := range c.users {
		if cUser.user.Id == userId {
			return &cUser	
		}
	}
	return nil
}

// remove a element from a slice and return the new slice
func removeFromSlice(s []channelUser, i int) []channelUser {
	s[i] = s[len(s) - 1] 
	return s[:len(s) - 1]
}

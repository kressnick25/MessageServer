package lib

import (
	"fmt"
	"sync"
)

type ChannelStore struct {
	mu sync.RWMutex
	channels map[int]*Channel
}

func (s *ChannelStore) AddNewChannel(channelId int) (*Channel, error) {
	s.mu.Lock()
	defer s.mu.Unlock()

	if s.getChannel(channelId) == nil {
		return nil, fmt.Errorf("Channel with id [%d] already exists", channelId)
	}

	newChannel := NewChannel(channelId)
	s.channels[channelId] = newChannel

	return newChannel, nil
}

func (s *ChannelStore) Subscribe(channelId int, user *User) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	s.getChannel(channelId).Subscribe(user)
}

func (s *ChannelStore) Unsubscribe(channelId int, user *User) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	s.getChannel(channelId).Unsubscribe(user.Id)

}

func (s *ChannelStore) UnsubscribeAll(user *User) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	channels := s.getChannelsForUser(user.Id)
	if channels == nil {
		return
	}

	for _, c := range channels {
		c.Unsubscribe(user.Id)
	}

}

func (s *ChannelStore) SendMessage(channelId int, user *User, message *Message) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	s.getChannel(channelId).Send(message)
}

func (s *ChannelStore) ReadNewMessages(channelId int, user *User) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	s.getChannel(channelId).GetNewMessages(user.Id)
}

func (s *ChannelStore) getChannel(channelId int) *Channel {
	return s.channels[channelId]
}

func (s *ChannelStore) getChannelsForUser(userId string) []*Channel {
	result := []*Channel{}
	for _, c := range s.channels {
		u := c.getChannelUser(userId)
		if u != nil {
			result = append(result, c)
		}
	}
	return result
}
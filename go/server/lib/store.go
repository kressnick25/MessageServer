package lib

import (
	"fmt"
	"sync"
)

type ChannelStore struct {
	mu sync.RWMutex
	channels map[int]*Channel
}

func NewStore() *ChannelStore {
	return &ChannelStore{sync.RWMutex{}, map[int]*Channel{}}
}

func (s *ChannelStore) AddNewChannel(channelId int) error {
	s.mu.Lock()
	defer s.mu.Unlock()

	if s.getChannel(channelId) != nil {
		return fmt.Errorf("Channel with id [%d] already exists", channelId)
	}

	newChannel := NewChannel(channelId)
	s.channels[channelId] = newChannel

	return nil
}

func (s *ChannelStore) DeleteChannel(channelId int) {
	s.mu.Lock()
	defer s.mu.Unlock()

	if s.getChannel(channelId) == nil {
		return
	}

	s.channels[channelId] = nil	
}


func (s *ChannelStore) Subscribe(channelId int, user *User) error {
	s.mu.RLock()
	defer s.mu.RUnlock()

	c := s.getChannel(channelId)
	if c == nil {
		return fmt.Errorf("Channel with id [%d] does not exist", channelId)
	}

	c.Subscribe(user)
	return nil
}

func (s *ChannelStore) Unsubscribe(channelId int, user *User) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	c := s.getChannel(channelId)
	if c == nil {
		return
	}
	
	c.Unsubscribe(user.Id)

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

func (s *ChannelStore) SendMessage(channelId int, message *Message) error {
	s.mu.RLock()
	defer s.mu.RUnlock()

	c := s.getChannel(channelId)
	if c == nil {
		return fmt.Errorf("Channel with id [%d] does not exist", channelId)
	}

	c.Send(message)
	return nil
}

func (s *ChannelStore) ReadNewMessages(channelId int, user *User) ([]*Message, error) {
	s.mu.RLock()
	defer s.mu.RUnlock()

	c := s.getChannel(channelId)
	if c == nil {
		return nil, fmt.Errorf("Channel with id [%d] does not exist", channelId)
	}

	return c.GetNewMessages(user.Id)
}

func (s *ChannelStore) getChannel(channelId int) *Channel {
	if s.channels[channelId] == nil {
		return nil
	}
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
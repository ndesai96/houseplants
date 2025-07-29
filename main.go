package main

import (
	"fmt"
	"log"
	"sync"
	"time"

	"github.com/eclipse/paho.mqtt.golang"
	"github.com/google/uuid"
)

var (
	id     = uuid.NewString()
	topic  = id + "/test"
	broker = "test.mosquitto.org:1883"
)

func main() {
	wg := &sync.WaitGroup{}

	messagesToPublish := 5
	wg.Add(messagesToPublish)

	ready := make(chan bool, 1)

	go runSubscriber(wg, ready)
	go runPublisher(messagesToPublish, ready)

	wg.Wait()

	fmt.Printf("Successfully published and received %d messages on topic: %s\n", messagesToPublish, topic)
}

func runSubscriber(wg *sync.WaitGroup, ready chan<- bool) {
	clientID := fmt.Sprintf("%s%d", "subscriber", time.Now().UnixNano())
	client := newClient(clientID)
	if err := connect(client); err != nil {
		log.Fatalf("Failed to connect subscriber to broker: %s - %s\n", broker, err)
	}
	defer client.Disconnect(250)

	messageHandler := func(client mqtt.Client, msg mqtt.Message) {
		fmt.Printf("Received \"%s\" on \"%s\"\n", msg.Payload(), msg.Topic())
		wg.Done()
	}

	if token := client.Subscribe(topic, 0, messageHandler); token.Wait() && token.Error() != nil {
		log.Fatalf("Failed to subscribe to topic: %s - %s\n", topic, token.Error())
	}

	ready <- true
	wg.Wait()
}

func runPublisher(count int, ready <-chan bool) {
	clientID := fmt.Sprintf("%s%d", "publisher", time.Now().UnixNano())
	client := newClient(clientID)
	if err := connect(client); err != nil {
		log.Fatalf("Failed to connect publisher to broker: %s - %s\n", broker, err)
	}
	defer client.Disconnect(250)

	<-ready
	for i := 0; i < count; i++ {
		message := fmt.Sprintf("Hello World %d", i+1)
		fmt.Printf("Publishing \"%s\" on \"%s\"\n", message, topic)
		if token := client.Publish(topic, 1, false, message); token.Wait() && token.Error() != nil {
			log.Fatalf("Failed to publish message: %v", token.Error())
		}
	}
}

func newClient(clientID string) mqtt.Client {
	opts := mqtt.NewClientOptions()
	opts.AddBroker(broker)
	opts.SetClientID(clientID)
	opts.SetOnConnectHandler(func(client mqtt.Client) {
		fmt.Printf("%s connected to broker: %s\n", clientID, broker)
	})
	opts.SetConnectionLostHandler(func(client mqtt.Client, err error) {
		log.Fatalf("%s lost connection to broker: %s - %s\n", clientID, broker, err)
	})

	return mqtt.NewClient(opts)
}

func connect(client mqtt.Client) error {
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		return token.Error()
	}
	return nil
}

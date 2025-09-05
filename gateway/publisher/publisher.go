package main

import (
	"fmt"
	"log"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

func main() {
	// Define broker and client options
	broker := "tcp://localhost:1883"
	opts := mqtt.NewClientOptions().AddBroker(broker)
	opts.SetClientID("go_publisher")

	// Create and connect the client
	client := mqtt.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		log.Fatalf("Failed to connect to broker: %v", token.Error())
	}
	fmt.Println("Successfully connected to MQTT broker")

	// Disconnect when the main function exits
	defer client.Disconnect(250)

	topic := "houseplants/living-room"
	message := "TESTING"

	// Loop to publish a message every second
	for {
		token := client.Publish(topic, 0, false, message)
		token.Wait() // Wait for publish to complete
		fmt.Printf("Published message to topic '%s'\n", topic)
		time.Sleep(1 * time.Second)
	}
}

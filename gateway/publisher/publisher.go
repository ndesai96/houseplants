package main

import (
	"encoding/json"
	"fmt"
	"log"
	"math/rand"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	pb "github.com/ndesai96/houseplants/protobuf"
)

var (
	topic = "houseplants/living-room"
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

	for {
		message, err := json.Marshal(generateMoistureData())
		if err != nil {
			log.Fatalf("Failed to marshal message: %v", err)
		}

		token := client.Publish(topic, 0, false, message)
		token.Wait() // Wait for publish to complete
		fmt.Printf("Published message to topic '%s'\n", topic)
		time.Sleep(1 * time.Second)
	}
}

func generateMoistureData() *pb.MoistureData {
	return &pb.MoistureData{
		Moisture:    randomize(0, 100),
		Temperature: randomize(100, 200),
		Light:       randomize(200, 300),
	}
}

func randomize(min, max int) int32 {
	return int32(rand.Intn(max-min+1) + min)
}

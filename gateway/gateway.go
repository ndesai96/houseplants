package main

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"log"
	"net/url"
	"os"
	"strings"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	tlsutil "github.com/ndesai96/houseplants/gateway/tls"
	pb "github.com/ndesai96/houseplants/protobuf"
	"google.golang.org/grpc"
)

var (
	collectorAddr  = os.Getenv("COLLECTOR_ADDR")
	mqttBrokerAddr = os.Getenv("MQTT_BROKER_ADDR")
	mqttClientID   = os.Getenv("MQTT_CLIENT_ID")
	mqttTopic      = os.Getenv("MQTT_TOPIC")
)

type closeFunc func() error

type gateway struct {
	collectorClient      pb.CollectorClient
	closeCollectorClient closeFunc
	mqttClient           mqtt.Client
}

func newGateway() *gateway {
	// Build gRPC Collector client
	conn, err := grpc.NewClient(
		collectorAddr,
		grpc.WithTransportCredentials(tlsutil.BuildTransportCredentials()),
	)
	if err != nil {
		log.Fatalf("Failed to create collector client: %s", err)
	}

	return &gateway{
		collectorClient:      pb.NewCollectorClient(conn),
		closeCollectorClient: conn.Close,
	}
}

func (g *gateway) start(ctx context.Context) {
	g.mqttClient = g.buildMQTTClient(ctx)

	if token := g.mqttClient.Connect(); token.Wait() && token.Error() != nil {
		log.Fatalf("failed to connect to MQTT broker: %s", token.Error())
	}
}

func (g *gateway) stop() {
	_ = g.closeCollectorClient()
	g.mqttClient.Disconnect(250)
}

func (g *gateway) buildMQTTClient(ctx context.Context) mqtt.Client {
	opts := mqtt.NewClientOptions()
	opts.AddBroker(mqttBrokerAddr)
	opts.SetClientID(mqttClientID)

	tlsConfig, err := tlsutil.BuildTLSConfig()
	if err != nil {
		log.Fatalf("Failed to build TLS config: %s", err)
	}
	opts.SetTLSConfig(tlsConfig)

	opts.SetAutoReconnect(true)
	opts.SetMaxReconnectInterval(5 * time.Second)

	opts.SetCleanSession(true)
	opts.SetKeepAlive(2 * time.Second)

	opts.SetConnectRetry(true)
	opts.SetConnectRetryInterval(5 * time.Second)

	opts.SetConnectionAttemptHandler(func(broker *url.URL, tlsCfg *tls.Config) *tls.Config {
		log.Printf("%s attempting to connect to MQTT broker: %s\n", mqttClientID, broker)
		return tlsCfg
	})
	opts.SetOnConnectHandler(func(client mqtt.Client) {
		log.Printf("%s connected to MQTT broker: %s\n", mqttClientID, mqttBrokerAddr)
		g.subscribe(ctx)
	})
	opts.SetConnectionLostHandler(func(client mqtt.Client, err error) {
		log.Printf("%s lost connection to MQTT broker: %s - %s\n", mqttClientID, mqttBrokerAddr, err)
	})

	return mqtt.NewClient(opts)
}

func (g *gateway) subscribe(ctx context.Context) {
	if token := g.mqttClient.Subscribe(mqttTopic, 0, g.mqttMessageHandler(ctx)); token.Wait() && token.Error() != nil {
		log.Fatalf("Failed to subscribe to MQTT topic: %s - %s\n", mqttTopic, token.Error())
	}
}

func (g *gateway) mqttMessageHandler(ctx context.Context) mqtt.MessageHandler {
	return func(client mqtt.Client, msg mqtt.Message) {
		topicPrefix := strings.TrimSuffix(mqttTopic, "#")
		deviceID := strings.TrimPrefix(msg.Topic(), topicPrefix)
		data, err := parseMoistureData(msg.Payload())
		if err != nil {
			log.Printf("Failed to parse moisture data: %s\n", err)
			return
		}

		if _, err = g.collectorClient.SendMoistureData(ctx, &pb.SendMoistureDataRequest{
			DeviceID: deviceID,
			Data:     data,
		}); err != nil {
			log.Printf("Failed to send moisture data to collector: %s\n", err)
		}
	}
}

func parseMoistureData(msg []byte) (*pb.MoistureData, error) {
	var data pb.MoistureData
	if err := json.Unmarshal(msg, &data); err != nil {
		return nil, err
	}
	return &data, nil
}

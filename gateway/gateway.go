package main

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"log"
	"net/url"
	"os"
	"strings"

	pb "github.com/ndesai96/houseplants/protobuf"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

var (
	collectorAddr  = os.Getenv("COLLECTOR_ADDR")
	clientCert     = os.Getenv("CLIENT_CERT")
	clientKey      = os.Getenv("CLIENT_KEY")
	caCertFile     = os.Getenv("CA_CERT")
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
	collectorClient, closeCollectorClient := buildCollectorClient()

	return &gateway{
		collectorClient:      collectorClient,
		closeCollectorClient: closeCollectorClient,
		mqttClient:           buildMQTTClient(),
	}
}

func (g *gateway) start(ctx context.Context) {
	if token := g.mqttClient.Connect(); token.Wait() && token.Error() != nil {
		log.Fatalf("failed to connect to MQTT broker: %s", token.Error())
	}

	if token := g.mqttClient.Subscribe(mqttTopic, 0, g.mqttMessageHandler(ctx)); token.Wait() && token.Error() != nil {
		log.Fatalf("Failed to subscribe to MQTT topic: %s - %s\n", mqttTopic, token.Error())
	}
}

func (g *gateway) mqttMessageHandler(ctx context.Context) mqtt.MessageHandler {
	return func(client mqtt.Client, msg mqtt.Message) {
		topicPrefix := strings.TrimSuffix(mqttTopic, "#")
		deviceID := strings.TrimPrefix(msg.Topic(), topicPrefix)
		if _, err := g.collectorClient.SendMoistureData(ctx, &pb.SendMoistureDataRequest{
			DeviceID: deviceID,
		}); err != nil {
			log.Printf("Failed to send moisture data to collector: %s\n", err)
		}
	}
}

func (g *gateway) stop() {
	_ = g.closeCollectorClient()
	g.mqttClient.Disconnect(250)
}

func buildCollectorClient() (pb.CollectorClient, closeFunc) {
	conn, err := grpc.NewClient(collectorAddr, grpc.WithTransportCredentials(buildTransportCredentials()))
	if err != nil {
		log.Fatalf("Failed to create collector client: %s", err)
	}

	return pb.NewCollectorClient(conn), conn.Close
}

func buildTransportCredentials() credentials.TransportCredentials {
	certificate, err := tls.LoadX509KeyPair(clientCert, clientKey)
	if err != nil {
		log.Fatalf("Failed to load client certificate: %s", err)
	}

	caCert, err := os.ReadFile(caCertFile)
	if err != nil {
		log.Fatalf("Failed to read CA certificate: %s", err)
	}

	caCertPool := x509.NewCertPool()
	if ok := caCertPool.AppendCertsFromPEM(caCert); !ok {
		log.Fatalf("Failed to append CA certificate to pool")
	}

	return credentials.NewTLS(&tls.Config{
		Certificates: []tls.Certificate{certificate},
		RootCAs:      caCertPool,
	})
}

func buildMQTTClient() mqtt.Client {
	opts := mqtt.NewClientOptions()
	opts.AddBroker(mqttBrokerAddr)
	opts.SetClientID(mqttClientID)
	opts.SetConnectionAttemptHandler(func(broker *url.URL, tlsCfg *tls.Config) *tls.Config {
		log.Printf("%s attempting to connect to MQTT broker: %s\n", mqttClientID, broker)
		return tlsCfg
	})
	opts.SetOnConnectHandler(func(client mqtt.Client) {
		log.Printf("%s connected to MQTT broker: %s\n", mqttClientID, mqttBrokerAddr)
	})
	opts.SetConnectionLostHandler(func(client mqtt.Client, err error) {
		log.Fatalf("%s lost connection to MQTT broker: %s - %s\n", mqttClientID, mqttBrokerAddr, err)
	})

	return mqtt.NewClient(opts)
}

package main

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"log"
	"math/rand"
	"net"
	"os"
	"time"

	influxdb2 "github.com/influxdata/influxdb-client-go/v2"
	"github.com/influxdata/influxdb-client-go/v2/api"
	pb "github.com/ndesai96/houseplants/protobuf"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
)

var (
	serverCert = os.Getenv("SERVER_CERT")
	serverKey  = os.Getenv("SERVER_KEY")
	caCertFile = os.Getenv("CA_CERT")
)

type server struct {
	pb.UnimplementedCollectorServer

	writer api.WriteAPI
}

func (s *server) SendMoistureData(_ context.Context, req *pb.SendMoistureDataRequest) (*pb.SendMoistureDataResponse, error) {
	p := influxdb2.NewPointWithMeasurement("houseplants").
		AddTag("device", req.DeviceID).
		AddField("moisture", randomize(0, 100)).
		AddField("temperature", randomize(100, 200)).
		AddField("light", randomize(200, 300)).
		SetTime(time.Now())
	s.writer.WritePoint(p)

	return &pb.SendMoistureDataResponse{}, nil
}

func randomize(min, max int) int {
	return rand.Intn(max-min+1) + min
}

func (s *server) start() {
	lis, err := net.Listen("tcp", ":50051")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	grpcServer := grpc.NewServer(grpc.Creds(buildTransportCredentials()))
	pb.RegisterCollectorServer(grpcServer, s)

	log.Println("Listening on port 50051...")
	if err = grpcServer.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}

func buildTransportCredentials() credentials.TransportCredentials {
	certificate, err := tls.LoadX509KeyPair(serverCert, serverKey)
	if err != nil {
		log.Fatalf("Failed to load server certificate: %s", err)
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
		ClientAuth:   tls.RequireAndVerifyClientCert,
		Certificates: []tls.Certificate{certificate},
		ClientCAs:    caCertPool,
	})
}

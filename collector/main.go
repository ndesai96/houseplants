package main

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"log"
	"net"
	"os"

	pb "github.com/ndesai96/houseplants/protobuf"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
)

var (
	serverCert = os.Getenv("SERVER_CERT")
	serverKey  = os.Getenv("SERVER_KEY")
	caCertFile = os.Getenv("CA_CERT")
)

func main() {
	lis, err := net.Listen("tcp", ":50051")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	s := grpc.NewServer(grpc.Creds(buildTransportCredentials()))
	pb.RegisterCollectorServer(s, &collector{})

	log.Println("Listening on port 50051...")
	if err = s.Serve(lis); err != nil {
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

type collector struct {
	pb.UnimplementedCollectorServer
}

func (c *collector) SendMoistureData(_ context.Context, req *pb.SendMoistureDataRequest) (*pb.SendMoistureDataResponse, error) {
	log.Println("Received: ", req)
	return &pb.SendMoistureDataResponse{}, nil
}

package main

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"log"
	"net"
	"os"
	"strconv"
	"time"

	influxdb2 "github.com/influxdata/influxdb-client-go/v2"
	"github.com/influxdata/influxdb-client-go/v2/api"
	pb "github.com/ndesai96/houseplants/protobuf"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/credentials"
	"google.golang.org/grpc/status"
)

var (
	serverCert     = os.Getenv("SERVER_CERT")
	serverKey      = os.Getenv("SERVER_KEY")
	caCertFile     = os.Getenv("CA_CERT")
	mTLSEnabled, _ = strconv.ParseBool(os.Getenv("MTLS_ENABLED"))
)

type server struct {
	pb.UnimplementedCollectorServer

	writer api.WriteAPI
}

func (s *server) SendMoistureData(_ context.Context, req *pb.SendMoistureDataRequest) (*pb.SendMoistureDataResponse, error) {
	if req.DeviceID == "" {
		return nil, status.Error(codes.InvalidArgument, "deviceID is empty")
	}
	if req.Data == nil {
		return nil, status.Error(codes.InvalidArgument, "data is empty")
	}
	p := influxdb2.NewPointWithMeasurement("houseplants").
		AddTag("device", req.DeviceID).
		AddField("moisture", req.Data.Moisture).
		AddField("temperature", req.Data.Temperature).
		AddField("light", req.Data.Light).
		SetTime(time.Now())
	s.writer.WritePoint(p)

	return &pb.SendMoistureDataResponse{}, nil
}

func (s *server) start() {
	lis, err := net.Listen("tcp", ":50051")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	var opts []grpc.ServerOption
	if mTLSEnabled {
		opts = append(opts, grpc.Creds(buildTransportCredentials()))
	}

	grpcServer := grpc.NewServer(opts...)
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

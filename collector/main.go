package main

import (
	"context"
	"log"
	"net"

	pb "github.com/ndesai96/houseplants/protobuf"
	"google.golang.org/grpc"
)

func main() {
	lis, err := net.Listen("tcp", ":50051")
	if err != nil {
		panic(err)
	}

	s := grpc.NewServer()
	pb.RegisterCollectorServer(s, &collector{})

	if err = s.Serve(lis); err != nil {
		panic(err)
	}

	log.Println("Listening on port 50051...")
}

type collector struct {
	pb.UnimplementedCollectorServer
}

func (c *collector) SendMoistureData(_ context.Context, req *pb.SendMoistureDataRequest) (*pb.SendMoistureDataResponse, error) {
	log.Println("Received: ", req)
	return &pb.SendMoistureDataResponse{}, nil
}

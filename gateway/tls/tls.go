package tls

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"log"
	"os"

	"google.golang.org/grpc/credentials"
)

var (
	clientCert = os.Getenv("CLIENT_CERT")
	clientKey  = os.Getenv("CLIENT_KEY")
	caCertFile = os.Getenv("CA_CERT")
)

func BuildTransportCredentials() credentials.TransportCredentials {
	tlsConfig, err := BuildTLSConfig()
	if err != nil {
		log.Fatalf("Failed to build TLS config: %s", err)
	}

	return credentials.NewTLS(tlsConfig)
}

func BuildTLSConfig() (*tls.Config, error) {
	certificate, err := tls.LoadX509KeyPair(clientCert, clientKey)
	if err != nil {
		return nil, fmt.Errorf("failed to load client certificate: %s", err)
	}

	caCert, err := os.ReadFile(caCertFile)
	if err != nil {
		return nil, fmt.Errorf("failed to read CA certificate: %s", err)
	}

	caCertPool, err := x509.SystemCertPool()
	if err != nil {
		log.Fatalf("failed to get system cert pool: %v", err)
	}

	if ok := caCertPool.AppendCertsFromPEM(caCert); !ok {
		return nil, fmt.Errorf("failed to append CA certificate to pool")
	}

	return &tls.Config{
		Certificates: []tls.Certificate{certificate},
		RootCAs:      caCertPool,
	}, nil
}

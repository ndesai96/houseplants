#!/bin/bash

# This script generates certificates and keys needed for mTLS.

mkdir -p certs

# Private keys for CA
openssl genrsa -out certs/ca.key 2048
openssl genrsa -out certs/server.key 2048

# Generate CA certificate
openssl req -new -x509 -nodes -days 1000 -key certs/ca.key -out certs/ca.crt -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=testServerCA"

# Generate a certificate signing request
openssl req -newkey rsa:2048 -nodes -keyout certs/client.key -out certs/client.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=client"
openssl req -newkey rsa:2048 -nodes -keyout certs/server.key -out certs/server.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=server"

# Have the CA sign the certificate requests and output the certificates.
openssl x509 -req -in certs/client.csr -days 398 -CA certs/ca.crt -CAkey certs/ca.key -set_serial 01 -out certs/client.crt
openssl x509 -req -in certs/server.csr -days 398 -CA certs/ca.crt -CAkey certs/ca.key -set_serial 01 -out certs/server.crt

# Clean up
rm certs/server.csr
rm certs/client.csr
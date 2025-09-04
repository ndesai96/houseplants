#!/bin/bash

# This script generates certificates and keys needed for mTLS.
# It accepts an optional argument for the server hostname.
# If no argument is provided, it defaults to "collector".
HOSTNAME=${1:-collector}

# --- Certificate Authority (CA) ---

# Private key for CA
openssl genrsa -out ca.key 2048

# Generate CA certificate
openssl req -new -x509 -nodes -days 1000 -key ca.key -out ca.crt -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=houseplants"

# --- Server Certificate ---

# Create a config file for the server certificate to add a Subject Alternative Name (SAN)
cat > server.ext << EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = ${HOSTNAME}
EOF

# Generate a certificate signing request (CSR) for the server.
openssl req -newkey rsa:2048 -nodes -keyout server.key -out server.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=${HOSTNAME}"

# Have the CA sign the server's CSR to create the server certificate.
openssl x509 -req -in server.csr -days 398 -CA ca.crt -CAkey ca.key -set_serial 01 -out server.crt -extfile server.ext

# --- Client Certificate ---

# Generate a certificate signing request (CSR) for the client
openssl req -newkey rsa:2048 -nodes -keyout client.key -out client.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=client"

# Have the CA sign the client's CSR to create the client certificate.
openssl x509 -req -in client.csr -days 398 -CA ca.crt -CAkey ca.key -set_serial 01 -out client.crt

# Clean up
rm server.csr
rm client.csr
rm server.ext
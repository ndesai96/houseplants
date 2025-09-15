#!/bin/bash

# This script generates certificates and keys needed for mTLS.
# It accepts an optional argument for the broker hostname.
BROKER_HOSTNAME=${1:-localhost}

# --- Gateway Certificate ---

# Generate a certificate signing request (CSR) for the gateway
openssl req -newkey rsa:2048 -nodes -keyout gateway.key -out gateway.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=gateway"

# Have the CA sign the gateway's CSR to create the gateway certificate.
openssl x509 -req -in gateway.csr -days 398 -CA ca.crt -CAkey ca.key -set_serial 02 -out gateway.crt

# --- MQTT Broker Certificate ---

# Create a config file for the broker certificate to add a Subject Alternative Name (SAN)
cat > broker.ext << EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = ${BROKER_HOSTNAME}
DNS.2 = mosquitto
DNS.3 = raspberrypi.local
EOF

# Generate a certificate signing request (CSR) for the broker
openssl req -newkey rsa:2048 -nodes -keyout broker.key -out broker.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=broker"

# Have the CA sign the broker's CSR to create the broker certificate.
openssl x509 -req -in broker.csr -days 398 -CA ca.crt -CAkey ca.key -set_serial 03 -out broker.crt -extfile broker.ext

rm gateway.csr
rm broker.csr
rm broker.ext
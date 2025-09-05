#!/bin/bash

# This script generates certificates and keys needed for mTLS.
# It accepts an optional argument for the collector and broker hostnames.
COLLECTOR_HOSTNAME=${1:-collector}
BROKER_HOSTNAME=${2:-mosquitto}

# --- Certificate Authority (CA) ---

# Private key for CA
openssl genrsa -out ca.key 2048

# Generate CA certificate
openssl req -new -x509 -nodes -days 1000 -key ca.key -out ca.crt -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=houseplants"

# --- Collector Certificate ---

# Create a config file for the collector certificate to add a Subject Alternative Name (SAN)
cat > collector.ext << EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = ${COLLECTOR_HOSTNAME}
DNS.2 = localhost
EOF

# Generate a certificate signing request (CSR) for the collector.
openssl req -newkey rsa:2048 -nodes -keyout collector.key -out collector.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=${HOSTNAME}"

# Have the CA sign the collector's CSR to create the collector certificate.
openssl x509 -req -in collector.csr -days 398 -CA ca.crt -CAkey ca.key -set_serial 01 -out collector.crt -extfile collector.ext

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
DNS.2 = localhost
DNS.3 = raspberrypi.local
EOF

# Generate a certificate signing request (CSR) for the broker
openssl req -newkey rsa:2048 -nodes -keyout broker.key -out broker.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=broker"

# Have the CA sign the broker's CSR to create the broker certificate.
openssl x509 -req -in broker.csr -days 398 -CA ca.crt -CAkey ca.key -set_serial 03 -out broker.crt -extfile broker.ext

# --- Sensor Certificate ---

# Generate a certificate signing request (CSR) for the sensor
openssl req -newkey rsa:2048 -nodes -keyout sensor.key -out sensor.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=sensor"

# Have the CA sign the sensor's CSR to create the sensor certificate.
openssl x509 -req -in sensor.csr -days 398 -CA ca.crt -CAkey ca.key -set_serial 04 -out sensor.crt

# Clean up
rm collector.csr
rm collector.ext
rm gateway.csr
rm broker.csr
rm broker.ext
rm sensor.csr

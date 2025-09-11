#!/bin/bash

# This script generates certificates and keys needed for mTLS.
# It accepts an optional argument for the collector hostname.
COLLECTOR_HOSTNAME=${1:-localhost}

# --- Collector Certificate ---

# Create a config file for the collector certificate to add a Subject Alternative Name (SAN)
cat > collector.ext << EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = ${COLLECTOR_HOSTNAME}
DNS.2 = collector
EOF

# Generate a certificate signing request (CSR) for the collector.
openssl req -newkey rsa:2048 -nodes -keyout collector.key -out collector.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=${HOSTNAME}"

# Have the CA sign the collector's CSR to create the collector certificate.
openssl x509 -req -in collector.csr -days 398 -CA ca.crt -CAkey ca.key -set_serial 01 -out collector.crt -extfile collector.ext

rm collector.csr
rm collector.ext
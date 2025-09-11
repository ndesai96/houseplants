#!/bin/bash

# --- Certificate Authority (CA) ---

# Private key for CA
openssl genrsa -out ca.key 2048

# Generate CA certificate
openssl req -new -x509 -nodes -days 1000 -key ca.key -out ca.crt -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=houseplants"
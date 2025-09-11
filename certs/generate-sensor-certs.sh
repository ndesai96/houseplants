#!/bin/bash

# --- Sensor Certificate ---

# Generate a certificate signing request (CSR) for the sensor
openssl req -newkey rsa:2048 -nodes -keyout sensor.key -out sensor.csr -subj "/C=US/ST=Texas/L=Austin/O=Houseplants/OU=Houseplants/CN=sensor"

# Have the CA sign the sensor's CSR to create the sensor certificate.
openssl x509 -req -in sensor.csr -days 398 -CA ca.crt -CAkey ca.key -set_serial 04 -out sensor.crt

rm sensor.csr
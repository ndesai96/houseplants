#!/bin/bash

# First, run `terraform apply` to provision the initial infrastructure
terraform apply -var provision_ecs=false -auto-approve

# Prompt the user to enter the InfluxDB API token
echo "Please create an InfluxDB API token and paste it here:"
read -r DB_API_TOKEN

# Now, run `terraform apply` again, this time providing the token
terraform apply -var provision_ecs=true -var influxdb_api_token="$DB_API_TOKEN" -auto-approve

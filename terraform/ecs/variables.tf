variable "app_image" {
  description = "The Docker image to deploy for the task."
  type        = string
}

variable "app_name" {
  description = "The name of the application."
  type        = string
}

variable "ca_cert" {
  description = "The CA cert used for client authentication when mTLS is enabled."
  type        = string
  default     = ""
}

variable "certificate_arn" {
  description = "ARN of the SSL certificate to use for HTTPS."
  type        = string
}

variable "hosted_zone_id" {
  description = "The ID of the Route 53 Hosted Zone where the record will be created."
  type        = string
}

variable "hosted_zone_name" {
  description = "The name of the Route 53 Hosted Zone."
  type        = string
}

variable "influxdb_api_token" {
  description = "The API token for the InfluxDB database."
  type        = string
  sensitive   = true
}

variable "influxdb_api_token_secret_arn" {
  description = "The ARN of the InfluxDB API token secret"
  type        = string
}

variable "influxdb_bucket" {
  description = "The bucket name for InfluxDB."
  type        = string
}

variable "influxdb_organization" {
  description = "The organization name for InfluxDB."
  type        = string
}

variable "influxdb_security_group_id" {
  description = "The security group ID of the Timestream InfluxDB instance."
  type        = string
}

variable "influxdb_url" {
  description = "The URL for the InfluxDB database."
  type        = string
}

variable "private_subnets" {
  description = "The IDs of the private subnets."
  type        = list(string)
}

variable "public_subnets" {
  description = "The IDs of the public subnets."
  type        = list(string)
}

variable "subdomain" {
  description = "The subdomain of the hosted zone."
  type        = string
}

variable "vpc_id" {
  description = "The ID of the VPC."
  type        = string
}
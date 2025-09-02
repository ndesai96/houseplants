# Timestream InfluxDB

variable "influxdb_username" {
  description = "The username for the InfluxDB user."
  type        = string
  default     = "admin"
}

variable "influxdb_password" {
  description = "The password for the InfluxDB user."
  type        = string
  sensitive   = true
}

variable "influxdb_bucket" {
  description = "The bucket name for InfluxDB."
  type        = string
  default     = "test-bucket"
}

variable "influxdb_organization" {
  description = "The organization name for InfluxDB."
  type        = string
  default     = "test-organization"
}

# ECS Fargate

variable "provision_ecs" {
  description = "If true, provisions the ECS module. Set to false to manually create InfluxDB API token first."
  type        = bool
  default     = true
}

variable "app_image" {
  description = "The Docker image to deploy for the task."
  type        = string
  default     = null

  validation {
    condition     = !var.provision_ecs || (var.app_image != null && var.app_image != "")
    error_message = "The 'app_image' variable must be set when 'provision_ecs' is true."
  }
}

variable "ca_cert" {
  description = "The CA cert used for client authentication when mTLS is enabled"
  type        = string
  default     = ""

  validation {
    condition     = !var.provision_ecs || (var.ca_cert != null && var.ca_cert != "")
    error_message = "The 'ca_cert' variable must be set when 'provision_ecs' is true."
  }
}

variable "certificate_arn" {
  description = "ARN of the SSL certificate to use for HTTPS"
  type        = string
  default     = null

  validation {
    condition     = !var.provision_ecs || (var.certificate_arn != null && var.certificate_arn != "")
    error_message = "The 'certificate_arn' variable must be set when 'provision_ecs' is true."
  }
}

variable "hosted_zone_id" {
  description = "The ID of the Route 53 Hosted Zone where the record will be created."
  type        = string
  default     = null

  validation {
    condition     = !var.provision_ecs || (var.hosted_zone_id != null && var.hosted_zone_id != "")
    error_message = "The 'hosted_zone_id' variable must be set when 'provision_ecs' is true."
  }
}

variable "hosted_zone_name" {
  description = "The name of the Route 53 Hosted Zone."
  type        = string
  default     = null

  validation {
    condition     = !var.provision_ecs || (var.hosted_zone_name != null && var.hosted_zone_name != "")
    error_message = "The 'hosted_zone_name' variable must be set when 'provision_ecs' is true."
  }
}

variable "influxdb_api_token" {
  description = "The API token for the InfluxDB database"
  type        = string
  sensitive   = true
  default     = null

  validation {
    condition     = !var.provision_ecs || (var.influxdb_api_token != null && var.influxdb_api_token != "")
    error_message = "The 'influxdb_api_token' variable must be set when 'provision_ecs' is true."
  }
}

variable "subdomain" {
  description = "The subdomain of the hosted zone."
  type        = string
}
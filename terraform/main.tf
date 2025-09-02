terraform {
  backend "s3" {
    bucket       = "neil-desai-terraform-states"
    key          = "go_fiber_hello_world/db/terraform.tfstate"
    region       = "us-east-1"
    use_lockfile = true
    encrypt      = true
  }

  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 6.5.0"
    }
  }
}

provider "aws" {
  region = "us-east-1"
}

locals {
  name = "go-fiber-hello-world"
}

module "vpc" {
  source = "git::https://github.com/ndesai96/tf-modules.git//aws/vpc"

  name     = local.name
  az_count = 3
}

module "timestream_influxdb" {
  source = "git::https://github.com/ndesai96/tf-modules.git//aws/timestream_influxdb"

  name                = local.name
  bucket              = var.influxdb_bucket
  username            = var.influxdb_username
  password            = var.influxdb_password
  organization        = var.influxdb_organization
  subnet_ids          = module.vpc.public_subnets
  publicly_accessible = true
  vpc_id              = module.vpc.id
  enable_admin_access = true
  admin_access_cidr   = "0.0.0.0/0"
}

module "ecs" {
  count = var.provision_ecs ? 1 : 0

  source = "./ecs"

  app_image                     = var.app_image
  app_name                      = local.name
  ca_cert                       = var.ca_cert
  certificate_arn               = var.certificate_arn
  hosted_zone_id                = var.hosted_zone_id
  hosted_zone_name              = var.hosted_zone_name
  influxdb_api_token            = var.influxdb_api_token
  influxdb_api_token_secret_arn = module.timestream_influxdb.api_token_secret_arn
  influxdb_security_group_id    = module.timestream_influxdb.security_group_id
  influxdb_url                  = "https://${module.timestream_influxdb.endpoint}:8086"
  influxdb_bucket               = var.influxdb_bucket
  influxdb_organization         = var.influxdb_organization
  vpc_id                        = module.vpc.id
  public_subnets                = module.vpc.public_subnets
  private_subnets               = module.vpc.private_subnets
  subdomain                     = var.subdomain
}
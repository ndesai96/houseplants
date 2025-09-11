data "aws_secretsmanager_secret" "db_api_token_secret" {
  arn = var.influxdb_api_token_secret_arn
}

resource "aws_secretsmanager_secret_version" "db_api_token" {
  secret_id     = data.aws_secretsmanager_secret.db_api_token_secret.id
  secret_string = var.influxdb_api_token
}

module "ecs_fargate" {
  source = "git::https://github.com/ndesai96/tf-modules.git//aws/ecs_fargate"

  app_count            = 1
  app_min_count        = 1
  app_max_count        = 6
  app_image            = var.app_image
  app_name             = var.app_name
  app_port             = 50051
  app_protocol         = "HTTP"
  app_protocol_version = "GRPC"
  app_envs = {
    INFLUXDB_URL    = var.influxdb_url
    INFLUXDB_BUCKET = var.influxdb_bucket
    INFLUXDB_ORG    = var.influxdb_organization
  }
  app_secrets = {
    INFLUXDB_TOKEN = var.influxdb_api_token_secret_arn
  }
  aws_region      = "us-east-1"
  ca_cert         = var.ca_cert
  certificate_arn = var.certificate_arn
  private_subnets = var.private_subnets
  public_subnets  = var.public_subnets
  enable_mtls     = true
  vpc_id          = var.vpc_id

  depends_on = [aws_secretsmanager_secret_version.db_api_token]
}

resource "aws_vpc_security_group_ingress_rule" "ingress" {
  description                  = "Allow access from the ECS tasks to the Timestream InfluxDB instance for ${var.app_name}"
  security_group_id            = var.influxdb_security_group_id
  referenced_security_group_id = module.ecs_fargate.security_group_id
  ip_protocol                  = "tcp"
  from_port                    = 8086
  to_port                      = 8086
}

module "route53" {
  source = "git::https://github.com/ndesai96/tf-modules.git//aws/route53"

  alias_name       = module.ecs_fargate.alb_dns_name
  alias_zone_id    = module.ecs_fargate.alb_zone_id
  hosted_zone_id   = var.hosted_zone_id
  hosted_zone_name = var.hosted_zone_name
  subdomain        = var.subdomain
}
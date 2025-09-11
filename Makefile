protoc:
	@echo "Generating Go files"
	cd protobuf && protoc --go_out=. --go-grpc_out=. \
		--go-grpc_opt=paths=source_relative --go_opt=paths=source_relative *.proto

collector:
	@echo "Starting collector"
	cd collector && \
		SERVER_CERT=../certs/collector.crt \
		SERVER_KEY=../certs/collector.key \
		CA_CERT=../certs/ca.crt \
		INFLUXDB_URL=http://localhost:8086 \
		INFLUXDB_TOKEN=1bf3923b-f692-4ed5-86df-2bcee192b72f \
		INFLUXDB_ORG=home \
		INFLUXDB_BUCKET=houseplants \
		MTLS_ENABLED=true \
		go run .

gateway:
	@echo "Starting gateway"
	cd gateway && \
		COLLECTOR_ADDR=localhost:50051 \
		CLIENT_CERT=../certs/gateway.crt \
		CLIENT_KEY=../certs/gateway.key \
		CA_CERT=../certs/ca.crt \
		MQTT_BROKER_ADDR=ssl://localhost:8883 \
		MQTT_CLIENT_ID=gateway \
		MQTT_TOPIC=houseplants/# \
		go run .

ca:
	@echo "Generate certificate authority"
	cd certs && ./generate-ca.sh

upload-certs:
	@echo "Uploading certificates to ESP-32"
	mkdir -p esp32/data
	cp certs/sensor.* esp32/data
	cp certs/ca.crt esp32/data
	cd esp32 && platformio run --target buildfs --environment esp32dev && platformio run --target uploadfs --environment esp32dev

docker:
	@echo "Building Docker images"
	docker build -t houseplants-gateway -f gateway/Dockerfile .
	docker build -t houseplants-collector -f collector/Dockerfile .

tidy:
	@echo "Running go mod tidy in collector, gateway, and protobuf"
	cd collector && go mod tidy
	cd gateway && go mod tidy
	cd protobuf && go mod tidy
	@echo "Running go work sync at root"
	go work sync

mock-sensor:
	@echo "Mocking moisture sensor"
	cd gateway/publisher && \
		CLIENT_CERT=../../certs/sensor.crt \
        CLIENT_KEY=../../certs/sensor.key \
        CA_CERT=../../certs/ca.crt \
		go run publisher.go

.PHONY: protoc collector gateway ca upload-certs docker tidy mock-sensor

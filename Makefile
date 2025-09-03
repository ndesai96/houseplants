protoc:
	@echo "Generating Go files"
	cd protobuf && protoc --go_out=. --go-grpc_out=. \
		--go-grpc_opt=paths=source_relative --go_opt=paths=source_relative *.proto

gateway:
	@echo "Starting gateway"
	cd gateway && \
		COLLECTOR_ADDR=localhost:50051 \
		CLIENT_CERT=../scripts/certs/client.crt \
		CLIENT_KEY=../scripts/certs/client.key \
		MQTT_BROKER_ADDR=localhost:1883 \
		MQTT_CLIENT_ID=gateway \
		MQTT_TOPIC=houseplants/# \
		go run .

.PHONY: protoc gateway

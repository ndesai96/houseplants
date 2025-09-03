protoc:
	@echo "Generating Go files"
	cd protobuf && protoc --go_out=. --go-grpc_out=. \
		--go-grpc_opt=paths=source_relative --go_opt=paths=source_relative *.proto

collector:
	@echo "Starting collector"
	cd collector && \
		go run .

gateway:
	@echo "Starting gateway"
	cd gateway && \
		COLLECTOR_ADDR=localhost:50051 \
		CLIENT_CERT=../certs/client.crt \
		CLIENT_KEY=../certs/client.key \
		MQTT_BROKER_ADDR=localhost:1883 \
		MQTT_CLIENT_ID=gateway \
		MQTT_TOPIC=houseplants/# \
		go run .

certs:
	@echo "Generating certificates"
	./scripts/gen-certs.sh

.PHONY: protoc collector gateway certs

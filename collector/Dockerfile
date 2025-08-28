# Build stage
FROM golang:1.25.0-alpine AS builder

# Set working directory
WORKDIR /app

# Install Protobuf compiler and Go plugins
RUN apk add --no-cache protobuf make
RUN go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
RUN go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@latest

# Copy go mod and sum files
COPY collector/go.mod collector/go.sum ./collector/
COPY protobuf/go.mod protobuf/go.sum ./protobuf/

# Download dependencies for each module
RUN cd collector && go mod download
RUN cd protobuf && go mod download

# Copy the source code
COPY . .

# Generate Go Protobuf code
RUN make protoc

# Build the collector application
WORKDIR /app/collector
ARG TARGETOS
ARG TARGETARCH
RUN GOOS=$TARGETOS GOARCH=$TARGETARCH go build -o main main.go

# Final stage
FROM alpine:latest

WORKDIR /app

# Copy the binary from builder
COPY --from=builder /app/collector/main .

# Expose port 50051
EXPOSE 50051

# Run the application
CMD ["./main"]
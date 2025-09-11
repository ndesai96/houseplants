# Build stage
FROM golang:1.25.0-alpine AS builder

# Set working directory
WORKDIR /app

# Install Protobuf compiler and Go plugins
RUN apk add --no-cache protobuf make ca-certificates
RUN go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
RUN go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@latest

# Copy go mod and sum files
COPY gateway/go.mod gateway/go.sum ./gateway/
COPY protobuf/go.mod protobuf/go.sum ./protobuf/

# Download dependencies for each module
RUN cd gateway && go mod download
RUN cd protobuf && go mod download

# Copy the source code
COPY . .

# Generate Go Protobuf code
RUN make protoc

# Build the gateway application
WORKDIR /app/gateway
ARG TARGETOS
ARG TARGETARCH
RUN CGO_ENABLED=0 GOOS=$TARGETOS GOARCH=$TARGETARCH go build -ldflags="-w -s" -o main .

# Final stage
FROM scratch

WORKDIR /app

# Copy the binary from builder
COPY --from=builder /app/gateway/main .

# Copy the CA certificates from the builder stage
COPY --from=builder /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/ca-certificates.crt

# Run the application
CMD ["./main"]
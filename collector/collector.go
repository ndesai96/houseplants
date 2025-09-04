package main

import (
	"os"

	influxdb "github.com/influxdata/influxdb-client-go/v2"
)

var (
	influxDBUrl    = os.Getenv("INFLUXDB_URL")
	influxDBToken  = os.Getenv("INFLUXDB_TOKEN")
	influxDBOrg    = os.Getenv("INFLUXDB_ORG")
	influxDBBucket = os.Getenv("INFLUXDB_BUCKET")
)

type collector struct {
	server       *server
	influxClient influxdb.Client
}

func newCollector() *collector {
	influxClient := influxdb.NewClient(influxDBUrl, influxDBToken)
	return &collector{
		server: &server{
			writer: influxClient.WriteAPI(influxDBOrg, influxDBBucket),
		},
		influxClient: influxClient,
	}
}

func (c *collector) start() {
	go c.server.start()
}

func (c *collector) stop() {
	c.influxClient.Close()
}

package main

import "context"

func main() {
	c := newCollector()
	ctx := context.Background()
	c.start()

	<-ctx.Done()
	c.stop()
}

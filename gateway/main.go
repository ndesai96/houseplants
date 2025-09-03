package main

import "context"

func main() {
	g := newGateway()
	ctx := context.Background()
	g.start(ctx)

	<-ctx.Done()
	g.stop()
}

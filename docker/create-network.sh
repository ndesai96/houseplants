#!/bin/bash

if ! docker network inspect houseplants-network >/dev/null 2>&1; then
  echo "Network 'houseplants-network' not found. Creating it..."
  docker network create houseplants-network
else
  echo "Network 'houseplants-network' already exists."
fi

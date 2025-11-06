#!/bin/bash

# Number of requests
NUM_REQUESTS=10000

for i in $(seq 1 $NUM_REQUESTS); do
  # Send request in background with unique message
  curl "http://localhost:4221/echo/message_$i" &
done

# Wait for all background jobs to finish
wait
echo "All requests completed."

#try thread pools
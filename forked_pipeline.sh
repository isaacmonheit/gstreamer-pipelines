#!/bin/bash

# Function to clean up pipelines on exit
cleanup() {
    echo "Stopping pipelines..."
    kill $MAIN_PIPELINE_PID $FILE_PIPELINE_PID $DISPLAY_PIPELINE_PID $UDP_PIPELINE_PID 2>/dev/null
    wait $MAIN_PIPELINE_PID $FILE_PIPELINE_PID $DISPLAY_PIPELINE_PID $UDP_PIPELINE_PID 2>/dev/null
    echo "Pipelines stopped."
}

# Trap SIGINT (Ctrl+C) and call cleanup
trap cleanup SIGINT

# Start the main pipeline
echo "Starting main pipeline..."
gst-launch-1.0 -e v4l2src ! videoconvert ! x264enc ! interpipesink name=sink1 sync=false async=false &
MAIN_PIPELINE_PID=$!
sleep 2  # Give the main pipeline some time to start

# Start the file saving pipeline
echo "Starting file saving pipeline..."
gst-launch-1.0 interpipesrc listen-to=sink1 ! qtmux ! filesink location=output.mp4 sync=false async=false &
FILE_PIPELINE_PID=$!
sleep 2  # Give the file pipeline some time to start

# Start the display pipeline
echo "Starting display pipeline..."
gst-launch-1.0 interpipesrc listen-to=sink1 ! avdec_h264 ! videoconvert ! autovideosink sync=false &
DISPLAY_PIPELINE_PID=$!
sleep 2  # Give the display pipeline some time to start

# Start the UDP streaming pipeline
echo "Starting UDP streaming pipeline..."
gst-launch-1.0 interpipesrc listen-to=sink1 ! udpsink host=127.0.0.1 port=5000 sync=false async=false &
UDP_PIPELINE_PID=$!
sleep 2  # Give the UDP pipeline some time to start

# Wait for the main pipeline to finish (or Ctrl+C)
wait $MAIN_PIPELINE_PID

# Call cleanup
cleanup

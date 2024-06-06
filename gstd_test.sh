#!/bin/bash

# Absolute path to the video location
VIDEO=$1

# Graceful cleanup upon CTRL-C
trap "gstd-client pipeline_delete p; exit" SIGHUP SIGINT SIGTERM

# Make sure there is no pipeline with this name already
gstd-client pipeline_delete p

gstd-client pipeline_create p playbin uri=file://$VIDEO

# Listen to the EOS messages
gstd-client bus_filter p eos

gstd-client pipeline_play p

# Wait for the message to perform seek
while true; do
    gstd-client bus_read p
    gstd-client event_seek p 1.0
done
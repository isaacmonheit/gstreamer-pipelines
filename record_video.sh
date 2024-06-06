#!/bin/bash

# Path to the video location
VIDEO=$/home/zonk/Gitbuh/gstreamer-pipelines/videos/test_video.mp4

# Make sure there is no pipeline with this name already
gstd-client pipeline_delete p

gstd-client pipeline_create p videotestsrc is-live=true pattern=ball ! x264enc ! mp4mux ! filesink location=$VIDEO
gstd-client pipeline_play p

# Wait for user interruption
echo "Press enter to finish file"
read

# Send the EOS event to avoid corruption
gstd-client event_eos p
gstd-client bus_filter p eos 
gstd-client bus_read p
gstd-client pipeline_delete p

echo "Video recorded to \"${VIDEO}\""
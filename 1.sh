#!/bin/bash

gst-launch-1.0 -v v4l2src device=/dev/video0 ! queue ! videoconvert ! queue ! x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! queue ! h264parse ! interpipesink name=encoder_sink sync=false async=false

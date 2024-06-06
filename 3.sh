#!/bin/bash

gst-launch-1.0 -v udpsrc port=5000 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! queue ! rtph264depay ! queue ! avdec_h264 ! queue ! videoconvert ! queue ! autovideosink

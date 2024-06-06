#!/bin/bash

# Decoder pipeline 1: interpipesrc -> avdec_h264 -> autovideosink
gst-launch-1.0 -v interpipesrc listen-to=encoder_sink ! videoconvert ! autovideosink

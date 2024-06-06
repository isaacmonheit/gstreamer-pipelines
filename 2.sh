#!/bin/bash

gst-launch-1.0 -v interpipesrc channel=encoder_sink ! queue ! h264parse ! queue ! rtph264pay ! queue ! udpsink host=127.0.0.1 port=5000 sync=false async=false

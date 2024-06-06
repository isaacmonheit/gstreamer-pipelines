#!/bin/bash

# Encode pipeline: videotestsrc -> x264enc -> interpipesink
gst-launch-1.0 -v videotestsrc ! videoconvert ! x264enc ! interpipesink name=encoder_sink 

#!/bin/bash

# File converter pipeline 2: interpipesrc -> filesink
gst-launch-1.0 interpipesrc listen-to=encoder_sink ! h264parse ! mp4mux ! filesink location=videosc/output.mp4

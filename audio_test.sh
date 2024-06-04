#!/bin/bash
gst-launch-1.0 -v audiotestsrc ! audioconvert ! audioresample ! autoaudiosink

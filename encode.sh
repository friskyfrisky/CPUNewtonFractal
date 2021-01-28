#!/bin/bash
cd frames
if ! command -v ffmpeg
then
	echo "install ffmpeg"
else
	cd frames
	ffmpeg -framerate 30 -y -i out%00d.jpg ../uwu.mp4
fi

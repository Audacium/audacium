#!/bin/bash
rm -rf builds
mkdir builds
cd builds
cmake -G "Unix Makefiles" -Daudacity_use_ffmpeg=loaded ../
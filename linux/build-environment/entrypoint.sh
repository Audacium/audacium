#!/usr/bin/env bash

conan --version

if [ ! -d "audacium" ]
then
    git clone https://github.com/audacium/audacium
fi

mkdir -p build

cd build

cmake_options=(
    -G "Unix Makefiles"
    -DCMAKE_BUILD_TYPE=Release
    -Daudacium_lib_preference=system # Change the libs default to 'system'
    -Daudacium_obey_system_dependencies=On # And force it!
    -Daudacium_use_wxwidgets=local # wxWidgets 3.1 is not available
    -Daudacium_use_expat=system
    -Daudacium_use_lame=system
    -Daudacium_use_sndfile=system
    -Daudacium_use_soxr=system
    -Daudacium_use_portaudio=local # System portaudio is not yet usable
    -Daudacium_use_sqlite=local # We prefer using the latest version of SQLite
    -Daudacium_use_ffmpeg=loaded
    -Daudacium_use_id3tag=system # This library has bugs, that are fixed in *local* version
    -Daudacium_use_mad=system # This library has bugs, that are fixed in *local* version
    -Daudacium_use_nyquist=local # This library is not available
    -Daudacium_use_vamp=local # The dev package for this library is not available
    -Daudacium_use_ogg=system 
    -Daudacium_use_vorbis=system
    -Daudacium_use_flac=system
    -Daudacium_use_lv2=system
    -Daudacium_use_midi=system
    -Daudacium_use_portmixer=local # This library is not available
    -Daudacium_use_portsmf=system
    -Daudacium_use_sbsms=local # We prefer using the latest version of sbsms
    -Daudacium_use_soundtouch=system
    -Daudacium_use_twolame=system
    -Daudacium_has_networking=yes 
    -Daudacium_use_curl=system
)

cmake "${cmake_options[@]}" ../audacium

exit_status=$?

if [ $exit_status -ne 0 ]; then
    exit $exit_status
fi

make -j`nproc`

cd bin/Release
mkdir -p "Portable Settings"

ls -la .

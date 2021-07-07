#!/bin/sh

lib="${0%/*}/lib/audacium"
share="${0%/*}/share/audacium"

export LD_LIBRARY_PATH="${lib}:${LD_LIBRARY_PATH}"
export AUDACITY_MODULES_PATH="${AUDACITY_MODULES_PATH}:${lib}/modules"
export AUDACITY_PATH="${AUDACITY_PATH}:${share}"

exec "${0%/*}/bin/audacium" "$@"

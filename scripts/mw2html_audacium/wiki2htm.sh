#!/bin/sh

helpdir="../../help/manual"
tmpdir_="../../help/temp"
srcuri="https://alphamanual.audaciumteam.org/man"

mkdir -p "${tmpdir_}"
python3 mw2html.py "${srcuri}" "${tmpdir_}" -s
mv "${tmpdir_}/alphamanual.audaciumteam.org" "${helpdir}"
rm -r "${tmpdir_}"

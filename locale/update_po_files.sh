#!/bin/sh
# Run this script with locale as the current directory
set -o errexit
echo ";; Recreating audacium.pot using .h, .cpp and .mm files"
for path in ../modules/mod-* ../libraries/lib-* ../include ../src ../crashreports ; do
   find $path -name \*.h -o -name \*.cpp -o -name \*.mm
done | LANG=c sort | \
sed -E 's/\.\.\///g' |\
xargs xgettext \
--no-wrap \
--default-domain=audacium \
--directory=.. \
--keyword=_ --keyword=XO --keyword=XC:1,2c --keyword=XXO --keyword=XXC:1,2c --keyword=XP:1,2 --keyword=XPC:1,2,4c \
--add-comments=" i18n" \
--add-location=file  \
--copyright-holder='Audacium Team' \
--package-name="audacium" \
--package-version='3.0.3' \
--msgid-bugs-address="audacium-translation@lists.sourceforge.net" \
--add-location=file -L C -o audacium.pot 
echo ";; Adding nyquist files to audacium.pot"
for path in ../plug-ins ; do find $path -name \*.ny -not -name rms.ny; done | LANG=c sort | \
sed -E 's/\.\.\///g' |\
xargs xgettext \
--no-wrap \
--default-domain=audacium \
--directory=.. \
--keyword=_ --keyword=_C:1,2c --keyword=ngettext:1,2 --keyword=ngettextc:1,2,4c \
--add-comments=" i18n" \
--add-location=file  \
--copyright-holder='Audacium Team' \
--package-name="audacium" \
--package-version='3.0.3' \
--msgid-bugs-address="audacium-translation@lists.sourceforge.net" \
--add-location=file -L Lisp -j -o audacium.pot 
if test "${AUDACITY_ONLY_POT:-}" = 'y'; then
    return 0
fi
echo ";; Updating the .po files - Updating Project-Id-Version"
for i in *.po; do
    sed -e '/^"Project-Id-Version:/c\
    "Project-Id-Version: audacium 3.0.3\\n"' $i > TEMP; mv TEMP $i
done
echo ";; Updating the .po files"
sed 's/.*/echo "msgmerge --lang=& &.po audacium.pot -o &.po";\
msgmerge --no-wrap --lang=& &.po audacium.pot -o &.po;/g' LINGUAS | bash
echo ";; Removing '#~|' (which confuse Windows version of msgcat)"
for i in *.po; do
    sed '/^#~|/d' $i > TEMP; mv TEMP $i
done
echo ""
echo ";;Translation updated"
echo ""
head -n 11 audacium.pot | tail -n 3
wc -l audacium.pot

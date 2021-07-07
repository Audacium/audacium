python3 mw2html.py https://alphamanual.audaciumteam.org/man ..\..\help\temp -s
rmdir /S /Q ..\..\help\manual
mkdir ..\..\help\manual
xcopy ..\..\help\temp\alphamanual.audaciumteam.org ..\..\help\manual\ /E /C /Y /Q
rmdir /S /Q ..\..\help\temp


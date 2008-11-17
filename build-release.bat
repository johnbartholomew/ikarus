
@echo off

SETLOCAL
SET VER=%1
SET VC="C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE\VCExpress.exe"

@echo Building latest version
%VC% vc90\ikarus.sln /build Release

@echo Exporting from git
git archive --format=tar --prefix=ikarus-%VER%/ HEAD | tar -x
rm pax_global_header
rm -rf ../ikarus-%VER%/

cp bin/ikarus.exe ikarus-%VER%/release/ikarus.exe
if exist report/Ikarus-jb5950.pdf (cp report/Ikarus-jb5950.pdf ikarus-%VER%/Ikarus-jb5950.pdf)
mv ikarus-%VER%/ ../ikarus-%VER%/

pushd ..

@echo Removing/renaming internal files
mv ikarus-%VER%\release ikarus-%VER%\windows
rm ikarus-%VER%\.gitignore
rm ikarus-%VER%\build-release.bat
rm -r ikarus-%VER%\report

@echo Building zip archive
rm -f ikarus-%VER%.zip
rm -f ikarus-%VER%-noprefix.zip
cd ikarus-%VER%
zip -rq ..\ikarus-%VER%-noprefix.zip *
cd ..
zip -rq ikarus-%VER%.zip ikarus-%VER%\

popd

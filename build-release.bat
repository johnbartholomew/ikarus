
@echo off

SET VER=%1

echo Building archive
git archive --format=tar --prefix=ikarus-%VER%/ HEAD > ..\ikarus-%VER%.tar

pushd ..

echo Extracting archive
tar -xf ikarus-%VER%.tar

echo Removing internal files
rm ikarus-%VER%\.gitignore
rm -r ikarus-%VER%\report
mv ikarus-%VER%\release ikarus-%VER%\windows
rm -r ikarus-%VER%\assets
rm -r ikarus-%VER%\bin
rm -r ikarus-%VER%\lib

popd

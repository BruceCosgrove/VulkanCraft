@echo off
pushd ..
"Dependencies/premake/premake5.exe" --file="BuildAll.lua" vs2022
popd

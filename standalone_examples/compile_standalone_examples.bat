@echo off
setlocal enabledelayedexpansion

set "failed_examples="
pushd "%~dp0"

for /d %%d in (*) do (
    pushd "%%d"
    mklink /d morphologica ..\..\..\morphologica
    if not exist build mkdir build
    pushd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg_repo\scripts\buildsystems\vcpkg.cmake
    make
    if errorlevel 1 (
        set "failed_examples=!failed_examples! %%d"
    )
    popd
    popd
)

echo.
if "!failed_examples!" == "" (
    echo All standalone examples have been built successfully!
    exit /b 0
) else (
    for %%i in (!failed_examples!) do (
        echo Standalone example %%i has failed...
    )
    exit /b 1
)

popd
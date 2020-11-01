@echo off
:: Stage pywinrt outputs into the solution output directory for 
:: usage <generated file folder> <binary target dir> <projectname>

echo Staging python package files

set SOLUTION_DIR="%~1"
set GENERATED_FILES="%~2"
set BIN_DIR="%~3"
set PROJECT_NAME="%~4"

mkdir %BIN_DIR%\%PROJECT_NAME%.wheel
mkdir %BIN_DIR%\%PROJECT_NAME%.wheel\winrt

:: Copy files built as part of the project - note that this doesn't copy the binary.
:: Depending on which build is running, it may need to be signed first.
copy %GENERATED_FILES%\pywinrt\winrt\__ini__.py %BIN_DIR%\%PROJECT_NAME%.wheel\winrt
xcopy /e %GENERATED_FILES%\pywinrt\winrt\windows %BIN_DIR%\%PROJECT_NAME%.wheel\winrt

copy %SOLUTION_DIR%\LICENSE %BIN_DIR%\%PROJECT_NAME%.wheel

:: To create a wheel,
:: copy setup.py int the wheel directory, then run
::   python setup.py bdist_wheel --python-tag cp39 --plat-name win-amd64

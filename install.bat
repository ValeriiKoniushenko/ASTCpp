@echo off

where python
if %ERRORLEVEL% NEQ 0 echo Error: python wasn't found. Install it(https://www.python.org/downloads/^) and try again. && pause && exit 1

where pip3
if %ERRORLEVEL% NEQ 0 echo Error: pip3 wasn't found. Install it(https://pypi.org/project/pip/^) and try again. && pause && exit 1

for /F "delims=" %%i in ('pip3 list ^| findstr packaging') do set "isExistsPipPackaging=%%i"
if "%isExistsPipPackaging%"=="" (
    echo Error: pip3's package: 'packaging' wasn't found. Install it(https://pypi.org/project/packaging/^) and try again.
    pause
    exit 1
)

where cppcheck
if %ERRORLEVEL% NEQ 0 echo Error: cppcheck wasn't found. Install it(https://sourceforge.net/projects/cppcheck/#download^) and try again. && pause && exit 1

where cmake
if %ERRORLEVEL% NEQ 0 echo Error: cmake wasn't found. Install it(https://cmake.org/download/^) and try again. && pause && exit 1

where git
if %ERRORLEVEL% NEQ 0 echo Error: git wasn't found. Install it(https://git-scm.com/downloads^) and try again.  && pause && exit 1

where curl
if %ERRORLEVEL% NEQ 0 echo Error: curl wasn't found. Install it(https://curl.se/download.html^) and try again.  && pause && exit 1

rem Check is in safe git repos
set isInGitSafeDir=0
for /F "tokens=*" %%i in ('git config list --global') do (
    if "%%i"=="safe.directory=%~dp0" (
        set isInGitSafeDir=1
        goto :foundGitSafeDir
    )
)

:foundGitSafeDir

if "%isInGitSafeDir%"=="0" (
    git config --global --add safe.directory %~dp0
)

rem Check and update git submodules
set count=0
for %%x in (dependencies/*) do set /a count+=1
if %ERRORLEVEL% NEQ 0 echo Error: Was met some error while trying to calculate dependecies && pause && exit 1

if %count%==0 (
    git submodule update --init --remote --recursive
    if %ERRORLEVEL% NEQ 0 echo Error: Was met some error while trying to updating git submodules\ && pause && exit 1
)
set rootCd=%cd%
cd scripts || (pause && exit 2)

set defaultCd=%cd%
install_boost.bat && (
    cd %defaultCd%

    if not exist check.py mklink check.py ..\dependencies\Utils\scripts\check.py

    if not exist file_validator.py mklink file_validator.py ..\dependencies\Utils\scripts\file_validator.py
    
    if not exist project_config.py mklink project_config.py ..\dependencies\Utils\scripts\project_config.py
    
    if not exist project_utils.py mklink project_utils.py ..\dependencies\Utils\scripts\project_utils.py
    
    if not exist project_validator.py mklink project_validator.py ..\dependencies\Utils\scripts\project_validator.py
    
    if not exist string_utils.py mklink string_utils.py ..\dependencies\Utils\scripts\string_utils.py
    
    if not exist git_hooks mklink /d git_hooks ..\dependencies\Utils\scripts\git_hooks
    
    python check.py --root %rootCd% && (
        echo Install is success!
    ) || (
        pause
        exit 1
    )

    ) || (
        echo Error: Was met some error while trying to install Boost
        pause
        exit 1
    )

pause
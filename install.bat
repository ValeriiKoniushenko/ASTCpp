@echo off

cd %~dp0

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
if not exist dependencies mkdir dependencies

set count=0
for %%x in (dependencies/*) do set /a count+=1
if %ERRORLEVEL% NEQ 0 echo Error: Was met some error while trying to calculate dependencies && pause && exit 1

if %count%==0 (
    git submodule update --init --remote --recursive
    if %ERRORLEVEL% NEQ 0 echo Error: Was met some error while trying to updating git submodules\ && pause && exit 1
)

dependencies/AST/install.bat
if %ERRORLEVEL% NEQ 0 echo Error: Was met some error while trying to install AST && pause && exit 1

set rootCd=%cd%
cd scripts || (echo Error: Can't cd to 'scripts' directory && pause && exit 2)

if not exist check.py mklink check.py ..\dependencies\AST\dependencies\Utils\scripts\check.py

if not exist file_validator.py mklink file_validator.py ..\dependencies\AST\dependencies\Utils\scripts\file_validator.py

if not exist project_config.py mklink project_config.py ..\dependencies\AST\dependencies\Utils\scripts\project_config.py

if not exist project_utils.py mklink project_utils.py ..\dependencies\AST\dependencies\Utils\scripts\project_utils.py

if not exist project_validator.py mklink project_validator.py ..\dependencies\AST\dependencies\Utils\scripts\project_validator.py

if not exist string_utils.py mklink string_utils.py ..\dependencies\AST\dependencies\Utils\scripts\string_utils.py

if not exist git_hooks mklink /d git_hooks ..\dependencies\AST\dependencies\Utils\scripts\git_hooks

python check.py --root %rootCd% && (
    echo Install is success!
) || (
    pause
    exit 1
)


pause
set START_DIR=%CD%
set SCRIPT_DIR=%~DP0

CD %SCRIPT_DIR%\..

git submodule update

CD %START_DIR%

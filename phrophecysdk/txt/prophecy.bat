@echo off

rem ===============================================
rem   configuration, install folder
rem ===============================================

set prophecy=d:\twilight\sdk\prophecy\

rem ===============================================
rem   automatic portion of the setup
rem ===============================================

set include=%include%;%prophecy%\include
set lib=%lib%;%prophecy%lib\visual studio .net 2003
echo Prophecy SDK: online.

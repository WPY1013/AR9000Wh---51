@echo off
start "" /wait "C:\Keil_v5\Keil_C51\UV4\UV4.exe" -r "%~dp0..\test.uvproj" -j0 -t "Target 1" -o "%~dp0uv4.log"
type "%~dp0uv4.log"

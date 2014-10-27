@echo off
WinSCP.com /command "option batch abort" "option confirm off" "open 3ds" "put %~dp0%1.3dsx /boot.3dsx" "exit"

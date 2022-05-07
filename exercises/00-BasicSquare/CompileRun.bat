@echo off
call MakefileWin.bat
for %%f in (*.exe) do start /b %%f

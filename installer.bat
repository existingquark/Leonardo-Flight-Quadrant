@echo off
echo Copying ThrottleQuadrant_Custom_v1.xml to MSFS InputProfiles folder...
xcopy "input_profiles\ThrottleQuadrant_Custom_v1.xml" "%LOCALAPPDATA%\Packages\Microsoft.FlightSimulator_8wekyb3d8bbwe\LocalState\InputProfiles\" /Y
echo Done!
pause


if not %1 == debug if not %1 == release goto InvalidArguments
if not %2 == x86 if not %2 == x64 goto InvalidArguments

set mode=Release
if %1 == debug set mode=Debug

set build_path=%2\%mode%\App
if %2 == x86 set build_path=Win32\%mode%\App

set lib_name=LicenseSpring
if %1 == debug set lib_name=LicenseSpringD

set libssl=libssl-3
if %2 == x64 set libssl=libssl-3-x64
set libcrypto=libcrypto-3
if %2 == x64 set libcrypto=libcrypto-3-x64

set lib_path=..\..\..\..\bin\%2\dynamic

echo Copy libs

if exist %build_path% (
 xcopy /s/y/i %lib_path%\%lib_name%.lib %build_path%
 xcopy /s/y/i %lib_path%\%lib_name%.dll %build_path%
 xcopy /s/y/i %lib_path%\%libssl%.dll %build_path%
 xcopy /s/y/i %lib_path%\libcurl.dll %build_path%
 xcopy /s/y/i %lib_path%\%libcrypto%.dll %build_path%
 xcopy /s/y/i %lib_path%\padlock.dll %build_path%
 xcopy /s/y/i %lib_path%\capi.dll %build_path%
)

exit /b 0

:InvalidArguments
echo Invalid arguments
echo Usage: %0 mode=<debug,release> arch=<x64,x86>
exit /b 1
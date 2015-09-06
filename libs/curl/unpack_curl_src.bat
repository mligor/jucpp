@echo off

set CURL_VERSION=7.44.0

if not exist "%~dp0\curl-src\README" (
"%~dp0..\..\tools\7za.exe" x curl-%CURL_VERSION%.zip -y -o"%~dp0\_curlsrc"
move "%~dp0\_curlsrc\curl-%CURL_VERSION%" "%~dp0\curl-src"
rmdir "%~dp0\_curlsrc"
)
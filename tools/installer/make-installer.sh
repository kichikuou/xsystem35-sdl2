#!/bin/sh
# To use this script in MSYS2 shell, install mingw-w64-x86_64-nsis and
# mingw-w64-x86_64-ntldd-git packages.

if [ $# -ne 2 ]; then
    echo 'Usage: make-installer.sh <build-dir> <nsi>'
    exit 1
fi

builddir=$1
nsi=$2

echo 'dlls.nsi:'
ntldd -R $builddir/src/xsystem35.exe |perl -ne 'print "File \"$1\"\n" if /=> (.*) \(/' |grep mingw |tee $builddir/dlls.nsi
echo

echo 'dlls-uninstall.nsi:'
sed 's/^File ".*\\/Delete "$INSTDIR\\/' $builddir/dlls.nsi |tee $builddir/dlls-uninstall.nsi
echo

makensis -INPUTCHARSET UTF8 -DBUILDDIR="$builddir" "$nsi"

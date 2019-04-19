#!/bin/sh

cat << !
This script will copy WICED wifi SDK from Wiced Studio 
installation. Then it adds MXCHIP and pico]OS patches to it.
!

#
# Get path to SDK directory.
#
if [ $# == 1 ]
then
	STUDIO_SDK="$1"
else
	echo -n "Enter path to Wiced Studio SDK directory:"
	read STUDIO_SDK
fi

WIFI_SDK=$STUDIO_SDK/43xxx_Wi-Fi
if [ ! -f $WIFI_SDK/version.txt ]
then
	echo "$STUDIO_SDK does not contain valid Wiced SDK."
	exit 1
fi

TARGET=`cat $WIFI_SDK/version.txt | awk '/^WICED Version:/ {

	match($0, "Wiced_([0-9]+)\\\\.([0-9]+)\\\\.([0-9]+)", v)
	printf("WICED-SDK-%d.%d\n", v[1], v[2]);
}'`

echo "Copying SDK to $PWD/$TARGET ..."

if [ -d $TARGET ]
then
	echo "Error: $TARGET already exists."
	exit 1
fi

mkdir $TARGET
cp -r $WIFI_SDK/. $TARGET

cd $TARGET

#
# Convert CRLF stuff with git.
#
echo "Fixing line endings..."
git init
git config core.autocrlf input
git add .
git commit -a -m "Initial commit."
git rm -r .
git reset --hard

#
# Get MXCHIP additions
#

git checkout -b mxchip
echo "Getting MXCHIP additions from github..."
git clone https://github.com/MXCHIP/MXCHIP-for-WICED

for BOARD in EMW3162 EMW3165 EMW3166
do
	echo "Copying platform $BOARD..."
	cp -r MXCHIP-for-WICED/patchs/$BOARD platforms/
	git add platforms/$BOARD
done

rm -rf MXCHIP-for-WICED
git commit -a -m "MXCHIP additions."

#
# pico]OS patching.
#
echo "Applying pico]OS patches..."

git checkout -b picoos
patch -p1 < ../wiced.patch
git commit -a -m "pico]OS patches added."

echo "Done! There is now a patched SDK in $TARGET."

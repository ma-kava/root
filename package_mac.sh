#!/bin/bash

IMAGE_NAME=PixetPro
BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DISTRIB_BUILD_DIR="$BASE_DIR/_build"
DMG_DISK_DIR="$BASE_DIR/_build/dmg"
PACKAGE_DIR="$DMG_DISK_DIR/Pixet.app"
PIXET_DIR="$DISTRIB_BUILD_DIR/Pixet"
API_DIR="$DISTRIB_BUILD_DIR/api"
CONFIG_DIR="$DISTRIB_BUILD_DIR/devices_configs"

PACKAGE_DMG="false"
PACKAGE_ZIP="false"
PACKAGE_EDU="false"
PACKAGE_API="false"

PLUGINS_SET="internal"
PIXET_VERSION=""
if [ -f "$BASE_DIR/src/common/ipixet.h" ]; then
   PIXET_VERSION=`grep PX_PIXET_VERSION "$BASE_DIR/src/common/ipixet.h" | grep -o "\".*\"" | sed 's/\"//g'`
fi

CHOSEN_ONE="false"
LIC_NAME="Advacam s.r.o."

while [ -n "$1" ]; do 
    case "$1" in
       dmg)
          PACKAGE_DMG="true"
          CHOSEN_ONE="true"
          ;;
       edu)
          PACKAGE_EDU="true"
          LIC_NAME="EDU"
          ;;
       api)
          PACKAGE_API="true"
          CHOSEN_ONE="true"
          ;;
       -lic)
          shift
          LIC_NAME="$1"
          ;;
       -pversion)
          shift
          PIXET_VERSION="$1"
          ;;
       -plugins)
          shift
          PLUGINS_SET="$1"
          ;;
       *)
          echo "Option $1 not recognized"
          exit -1
     esac
     shift
done
          
if [ "$CHOSEN_ONE" = "false" ]; then
   SCRIPT=`basename "$0"`
   echo ""
   echo "$SCRIPT [dmg][edu][api][-lic *name*][-pversion PIXET_VERSION]"
   echo ""
   echo "Package Pixel Build."
   echo "Multiple targets can be specified, at least one [dmg][api] must be chosen."
   echo ""
   exit -1		
fi

if [ -z "$PIXET_VERSION" ]; then 
   echo ""
   echo "Pixet version has to be set by -pversion parameter"
   echo ""
   exit -1
fi

rm -rf "${DMG_DISK_DIR}"
rm -rf "${PIXET_DIR}"

python3 ./generate_license.py "$LIC_NAME"


if [ "$PACKAGE_DMG" = "true" ]; then

	echo ===================================
	echo Building dmg package

    rm -f "${IMAGE_NAME}.dmg"

    mkdir -vp "${PACKAGE_DIR}/Contents/MacOS" 
    mkdir -vp "${PACKAGE_DIR}/Contents/Resources"
   
    unzip "${DISTRIB_BUILD_DIR}/Pixet_Pro*" -d  "${PIXET_DIR}"

    python "purge_pixet.py" --build-dir ${PIXET_DIR} --version ${PLUGINS_SET} --platform Darwin_x64_ARM64 --xml-config plugin_cookbook.xml 

	if [ "$PACKAGE_EDU" = "true" ]; then
		sed -i '' 's/;MainUi=devcontrol/MainUi=eduview/' "${PIXET_DIR}/pixet.ini"
	fi

	if [ -d "${CONFIG_DIR}" ] && [ "$(ls -A "${CONFIG_DIR}")" ]; then
		mkdir "${PACKAGE_DIR}/Contents/configs"
		mkdir "${PACKAGE_DIR}/Contents/factory"
                ls "${CONFIG_DIR}"
		cp -r "${CONFIG_DIR}/." "${PACKAGE_DIR}/Contents/configs/"
		cp -r "${CONFIG_DIR}/." "${PACKAGE_DIR}/Contents/factory/"
	fi

    cp "${PIXET_DIR}"/* "${PACKAGE_DIR}"/Contents/
    cp -R "${PIXET_DIR}"/Pixet.app/Contents/* "${PACKAGE_DIR}"/Contents/
    cp -R "${PIXET_DIR}"/hwlibs "${PACKAGE_DIR}"/Contents/
    cp -R "${PIXET_DIR}"/plugins/* "${PACKAGE_DIR}"/Contents/PlugIns/
    #mv "${PACKAGE_DIR}"/Contents/plugins "${PACKAGE_DIR}"/Contents/PlugIns
    cp -R "${PIXET_DIR}"/libs "${PACKAGE_DIR}"/Contents/
    cp -R "${PIXET_DIR}"/help "${PACKAGE_DIR}"/Contents/
    cp -R "${PIXET_DIR}"/samples "${PACKAGE_DIR}"/Contents/
    cp -R "${PIXET_DIR}"/scripts "${PACKAGE_DIR}"/Contents/
  
    cp lic.info "${PACKAGE_DIR}"/Contents
    
    ln -s /Applications/ "${DMG_DISK_DIR}"/Applications
    
    # sign
    # os.system("bash \"" + srcDir + "/sign.sh\" \"" + pixetProApp + "\"")

    #sips -i ${DISTRIB_BUILD_DIR}/pixetdrive.icns

    rm "${DISTRIB_BUILD_DIR}/${IMAGE_NAME}.dmg"
    hdiutil create -volname "${IMAGE_NAME}" -srcfolder "${DMG_DISK_DIR}" -ov -format UDRW "${DISTRIB_BUILD_DIR}/${IMAGE_NAME}_temp.dmg"
    FULL_MOUNT_OUTPUT=$(hdiutil attach -readwrite -noverify "${DISTRIB_BUILD_DIR}/${IMAGE_NAME}_temp.dmg")
    #echo "Mount info: ${FULL_MOUNT_OUTPUT}"
    MOUNT_POINT=$(echo "${FULL_MOUNT_OUTPUT}" | tail -1 | awk '{print $NF}')
    #echo "Mount point: ${MOUNT_POINT}"   

    mkdir "${MOUNT_POINT}/.background"
    cp "${BASE_DIR}/mac/dmgimage.png" "${MOUNT_POINT}/.background/dmgimage.png"
    SetFile -a V "${MOUNT_POINT}/.background"
    SetFile -a V "${MOUNT_POINT}/.fseventsd"
    #sips -m /System/Library/ColorSync/Profiles/sRGB\ Profile.icc "${MOUNT_POINT}/.background/dmgimage.png"
    #sips -m /System/Library/ColorSync/Profiles/Generic\ Gray\ Gamma\ 2.2\ Profile.icc "${MOUNT_POINT}/.background/dmgimage.png"
osascript << EOF
tell application "Finder"
   tell disk "${IMAGE_NAME}"
      open
      set current view of container window to icon view
      set toolbar visible of container window to false
      set statusbar visible of container window to false
      set the bounds of container window to {400, 100, 1000, 520}
      set theViewOptions to the icon view options of container window
      set arrangement of theViewOptions to not arranged
      set icon size of theViewOptions to 80
      set background picture of theViewOptions to file ".background:dmgimage.png"
      set position of item "Pixet.app" of container window to {200, 130}
      set position of item "Applications" of container window to {400, 130}
      update without registering applications
      delay 5
      close
   end tell
end tell
EOF

    sync

    cp "${BASE_DIR}/mac/pixetdrive.icns" "${MOUNT_POINT}/.VolumeIcon.icns"
    SetFile -c icnC "${MOUNT_POINT}/.VolumeIcon.icns"
    SetFile -a V "${MOUNT_POINT}/.VolumeIcon.icns"
    SetFile -a C "${MOUNT_POINT}"

    hdiutil detach "${MOUNT_POINT}"
    hdiutil convert "${DISTRIB_BUILD_DIR}/${IMAGE_NAME}_temp.dmg" -format UDZO -imagekey zlib-level=9 -o "${DISTRIB_BUILD_DIR}/${IMAGE_NAME}.dmg"
    rm "${DISTRIB_BUILD_DIR}/${IMAGE_NAME}_temp.dmg"


fi

if [ "$PACKAGE_API" = "true" ]; then
	 echo ===================================
	 echo Building zip API package

    rm -rf "${DISTRIB_BUILD_DIR}/Pixet_API.zip"
    rm -rf "${API_DIR}"    
    mkdir -p "${API_DIR}"

    unzip "${DISTRIB_BUILD_DIR}"/Pixet_API* -d  "${API_DIR}"
    cp lic.info "${API_DIR}" 

    zip -j "${DISTRIB_BUILD_DIR}"/Pixet_API.zip "${API_DIR}"/*

    rm -rf "${API_DIR}"

fi

rm -rf lic.info

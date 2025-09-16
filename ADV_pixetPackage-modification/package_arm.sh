#!/bin/bash

# docker run --rm --user 1001:1001 -v ${workspacePath}:/opt/Pixet:Z -t ubuntu20.04dev_64 bash -c \"cd /opt/Pixet; bash ./package_linux.sh deb\"

BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DISTRIB_BUILD_DIR=$BASE_DIR/_build
API_DIR=$DISTRIB_BUILD_DIR/api

PACKAGE_API="false"
ARCHITECTURE="ARM64"

PIXET_VERSION=""
if [ -f "$BASE_DIR/src/common/ipixet.h" ]; then
   PIXET_VERSION=`grep PX_PIXET_VERSION "$BASE_DIR/src/common/ipixet.h" | grep -oP "\".*\"" | sed 's/\"//g'`
fi

CHOSEN_ONE="false"
LIC_NAME="Advacam s.r.o."

while [ -n "$1" ]; do 
    case "$1" in
       arm32)
	      ARCHITECTURE="ARM32"
		  ;;
	   arm64)
	      ARCHITECTURE="ARM64"
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
       *)
          echo "Option $1 not recognized"
          exit -1
     esac
     shift
done
          
if [ "$CHOSEN_ONE" = "false" ] | [ "$CHOSEN_ARCHITECTURE" = "false" ]; then
   SCRIPT=`basename "$0"`
   echo ""
   echo "$SCRIPT [arm32|arm64][api][-lic *name*][-pversion PIXET_VERSION]"
   echo ""
   echo "Package Pixel Build."
   echo "Multiple targets can be specified, at least one [api] must be chosen."
   echo ""
   exit -1		
fi

if [ -z "$PIXET_VERSION" ]; then 
   echo ""
   echo "Pixet version has to be set by -pversion parameter"
   echo ""
   exit -1
fi

zip_files=(${DISTRIB_BUILD_DIR}/*Linux_${ARCHITECTURE}*.zip)
if [ ! -f "${zip_files[0]}" ]; then 
	echo ""
	echo "no pixet zip files for ${ARCHITECTURE} in ${DISTRIB_BUILD_DIR} directory"
	echo ""
	echo -1
fi

rm -rf ${API_DIR}
rm -f ${DISTRIB_BUILD_DIR}/Pixet_API_${ARCHITECTURE}.*

./generate_license.py "$LIC_NAME"
#mv lic.info ${PACKAGE_DIR} 

if [ "$PACKAGE_API" = "true" ]; then
	echo ===================================
	echo Building tar.gz API package

	mkdir -p ${API_DIR}

	unzip  "${DISTRIB_BUILD_DIR}/*${ARCHITECTURE}*" -d  ${API_DIR}
	cp lic.info ${API_DIR}
	cd ${API_DIR}

	tar -czf ${DISTRIB_BUILD_DIR}/Pixet_API_Linux_${ARCHITECTURE}.tar.gz *

	cd ${BASE_DIR}
    rm -rf ${API_DIR}
fi

# rm -rf ${PACKAGE_DIR}

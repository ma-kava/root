#!/bin/bash

# docker run --rm --user 1001:1001 -v ${workspacePath}:/opt/Pixet:Z -t ubuntu20.04dev_64 bash -c \"cd /opt/Pixet; bash ./package_linux.sh deb\"

BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DISTRIB_BUILD_DIR=$BASE_DIR/_build
PACKAGE_DIR=$DISTRIB_BUILD_DIR/package
CONFIG_DIR=$DISTRIB_BUILD_DIR/devices_configs

PACKAGE_DEB="false"
PACKAGE_RPM="false"
PACKAGE_TARGZ="false"
PACKAGE_EDU="false"
PACKAGE_API="false"

PIXET_VERSION=""
if [ -f "$BASE_DIR/src/common/ipixet.h" ]; then
   PIXET_VERSION=`grep PX_PIXET_VERSION "$BASE_DIR/src/common/ipixet.h" | grep -oP "\".*\"" | sed 's/\"//g'`
fi

PLUGINS_SET="internal"

CHOSEN_ONE="false"
LIC_NAME="Advacam s.r.o."

while [ -n "$1" ]; do 
    case "$1" in
       deb)
          PACKAGE_DEB="true"
          CHOSEN_ONE="true"
          ;;
       rpm)
          PACKAGE_RPM="true"
          CHOSEN_ONE="true"
          ;;
       targz)
          PACKAGE_TARGZ="true"
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
   echo "$SCRIPT [deb][rpm][targz][edu][api][-lic *name*][-pversion PIXET_VERSION]"
   echo ""
   echo "Package Pixel Build."
   echo "Multiple targets can be specified, at least one [deb][rpm][targz] must be chosen."
   echo ""
   exit -1		
fi

if [ -z "$PIXET_VERSION" ]; then 
   echo ""
   echo "Pixet version has to be set by -pversion parameter"
   echo ""
   exit -1
fi

zip_files=(${DISTRIB_BUILD_DIR}/*.zip)
if [ ! -f "${zip_files[0]}" ]; then 
	echo ""
	echo "no pixet zip files in ${DISTRIB_BUILD_DIR} directory"
	echo ""
	echo -1
fi

rm -rf ${PACKAGE_DIR}
rm -f ${DISTRIB_BUILD_DIR}/pixet.*

cp -r ${BASE_DIR}/linux/package ${DISTRIB_BUILD_DIR}

./generate_license.py "$LIC_NAME"
mv lic.info ${PACKAGE_DIR} 

if [ "$PACKAGE_DEB" = "true" ]; then

	echo ===================================
	echo Building Debian package

	mv ${PACKAGE_DIR}/deb/pixet/DEBIAN/control.template ${PACKAGE_DIR}/deb/pixet/DEBIAN/control
	`sed -i "s/#PIXET_VERSION#/${PIXET_VERSION}/g" ${PACKAGE_DIR}/deb/pixet/DEBIAN/control`

	DEB_PIXET_DIR=${PACKAGE_DIR}/deb/pixet/opt/pixet
	mkdir -p ${DEB_PIXET_DIR}
	unzip "${DISTRIB_BUILD_DIR}/Pixet_Pro*" -d  ${DEB_PIXET_DIR}

	python "purge_pixet.py" --plugin-dir ${DEB_PIXET_DIR}/plugins --version ${PLUGINS_SET} --platform Linux_x64 --xml-config plugin_cookbook.xml

	if [ "$PACKAGE_EDU" = "true" ]; then
		sed -i 's/;MainUi=devcontrol/MainUi=eduview/' ${DEB_PIXET_DIR}/pixet.ini
	fi

	cp -f ${PACKAGE_DIR}/lic.info ${DEB_PIXET_DIR}
	
	if [ -d "${CONFIG_DIR}" ] && [ "$(ls -A ${CONFIG_DIR})" ]; then
		mkdir ${DEB_PIXET_DIR}/configs
		mkdir ${DEB_PIXET_DIR}/factory
		cp -r ${CONFIG_DIR}/* ${DEB_PIXET_DIR}/configs/
		cp -r ${CONFIG_DIR}/* ${DEB_PIXET_DIR}/factory/
	fi

	chmod +x ${DEB_PIXET_DIR}/pixet.sh
	chmod +x ${DEB_PIXET_DIR}/Pixet
	chmod 0755 ${PACKAGE_DIR}/deb/pixet/DEBIAN
	cd ${PACKAGE_DIR}/deb

	dpkg --build pixet

	cp pixet.deb ${DISTRIB_BUILD_DIR}

	rm -rf ${DEB_PIXET_DIR}

	echo Debian Package Built

	cd ${BASE_DIR}
fi

if [ "$PACKAGE_RPM" = "true" ]; then
	echo ===================================
	echo Building RPM package

	cp ${PACKAGE_DIR}/rpm/pixet/SPECS/pixet.spec.template ${PACKAGE_DIR}/rpm/pixet/SPECS/pixet.spec
	`sed -i "s/#PIXET_VERSION#/${PIXET_VERSION}/g" ${PACKAGE_DIR}/rpm/pixet/SPECS/pixet.spec`

	RMP_PIXET_DIR=${PACKAGE_DIR}/rpm/pixet/BUILDROOT/pixet/opt/pixet
	mkdir -p ${RMP_PIXET_DIR}
	unzip  "${DISTRIB_BUILD_DIR}/Pixet_Pro*" -d  ${RMP_PIXET_DIR}

	python "purge_pixet.py" --plugin-dir ${RMP_PIXET_DIR}/plugins --version ${PLUGINS_SET} --platform Linux_x64 --xml-config plugin_cookbook.xml

	if [ "$PACKAGE_EDU" = "true" ]; then
		sed -i 's/;MainUi=devcontrol/MainUi=eduview/' ${RMP_PIXET_DIR}/pixet.ini
	fi

	cp -f ${PACKAGE_DIR}/lic.info ${RMP_PIXET_DIR}
	
	if [ -d "${CONFIG_DIR}" ] && [ "$(ls -A ${CONFIG_DIR})" ]; then
		mkdir ${RMP_PIXET_DIR}/configs
		mkdir ${RMP_PIXET_DIR}/factory
		cp -r ${CONFIG_DIR}/* ${RMP_PIXET_DIR}/configs/
		cp -r ${CONFIG_DIR}/* ${RMP_PIXET_DIR}/factory/
	fi

	chmod +x ${RMP_PIXET_DIR}/pixet.sh
	chmod +x ${RMP_PIXET_DIR}/Pixet

	cd ${PACKAGE_DIR}/rpm/pixet

	bash ./build.sh
	
	mv Pixet.rpm pixet.rpm
	cp pixet.rpm ${DISTRIB_BUILD_DIR}
	
	rm -rf ${RMP_PIXET_DIR}

	echo RPM Package Built

	cd ${BASE_DIR}
fi

if [ "$PACKAGE_TARGZ" = "true" ]; then
	echo ===================================
	echo Building tar.gz package

	ZIP_PIXET_DIR=${PACKAGE_DIR}/zip/pixet
	mkdir -p ${ZIP_PIXET_DIR}
	unzip  "${DISTRIB_BUILD_DIR}/Pixet_Pro*" -d  ${ZIP_PIXET_DIR}

	python "purge_pixet.py" --plugin-dir ${ZIP_PIXET_DIR}/plugins --version ${PLUGINS_SET} --platform Linux_x64 --xml-config plugin_cookbook.xml

	if [ "$PACKAGE_EDU" = "true" ]; then
		sed -i 's/;MainUi=devcontrol/MainUi=eduview/' ${ZIP_PIXET_DIR}/pixet.ini
	fi

	sed -i 's/UseAppDataDir=true/UseAppDataDir=false/' ${ZIP_PIXET_DIR}/pixet.ini

	cp -f ${PACKAGE_DIR}/lic.info ${ZIP_PIXET_DIR}
	
	if [ -d "${CONFIG_DIR}" ] && [ "$(ls -A ${CONFIG_DIR})" ]; then
		mkdir ${ZIP_PIXET_DIR}/configs
		mkdir ${ZIP_PIXET_DIR}/factory
		cp -r ${CONFIG_DIR}/* ${ZIP_PIXET_DIR}/configs/
		cp -r ${CONFIG_DIR}/* ${ZIP_PIXET_DIR}/factory/
	fi

	chmod +x ${ZIP_PIXET_DIR}/pixet.sh
	chmod +x ${ZIP_PIXET_DIR}/Pixet
	cd ${PACKAGE_DIR}/zip/

	tar -czf pixet.tar.gz pixet

	cp pixet.tar.gz ${DISTRIB_BUILD_DIR}

	rm -rf ${ZIP_PIXET_DIR}

	echo tar.gz Package Built

	cd ${BASE_DIR}
fi

if [ "$PACKAGE_API" = "true" ]; then
	echo ===================================
	echo Building tar.gz API package

	API_DIR=${PACKAGE_DIR}/api
	mkdir -p ${API_DIR}
	unzip  "${DISTRIB_BUILD_DIR}/Pixet_API*" -d  ${API_DIR}

	python "purge_pixet.py" --plugin-dir ${API_DIR}/plugins --version ${PLUGINS_SET} --platform Linux_x64 --xml-config plugin_cookbook.xml
	
	cp ${PACKAGE_DIR}/lic.info ${API_DIR}
	cd ${API_DIR}
	
	tar -czf ${DISTRIB_BUILD_DIR}/pixetAPI.tar.gz *
	
	cd ${BASE_DIR}
    rm -rf ${API_DIR}
fi

# rm -rf ${PACKAGE_DIR}

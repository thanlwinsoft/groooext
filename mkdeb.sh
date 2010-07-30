DEB_DIR=../grooodeb`uname -m`/
ARCHIVE_DIR=../grooodeb
PKG_VER=`tail -1 .hgtags | sed "s/.* //"`
SRC_DIR=`pwd`
export QUILT_PATCHES=debian/patches
mkdir -p ${ARCHIVE_DIR}
mkdir -p ${DEB_DIR}
rm -rf ${DEB_DIR}*
if test -f ${ARCHIVE_DIR}/openoffice.org-graphite_${PKG_VER}.orig.tar.gz;
then
    cp ${ARCHIVE_DIR}/openoffice.org-graphite_${PKG_VER}.orig.tar.gz ${DEB_DIR}
else
    hg archive -X debian -X .hgtags -p openoffice.org-graphite_${PKG_VER} -ttgz ${ARCHIVE_DIR}/openoffice.org-graphite_${PKG_VER}.orig.tar.gz
    cp ${ARCHIVE_DIR}/openoffice.org-graphite_${PKG_VER}.orig.tar.gz ${DEB_DIR}
fi
cd ${DEB_DIR}
tar -zxf openoffice.org-graphite_${PKG_VER}.orig.tar.gz
cd openoffice.org-graphite_${PKG_VER}
cp -R ${SRC_DIR}/debian debian
while quilt push; do echo; done
debuild
cd ..
lintian --pedantic -i openoffice.org-graphite_${PKG_VER}*.changes


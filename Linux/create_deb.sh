#!/bin/bash

declare -a Numbers=()

VersionString=""
while read p; do
  Number=$(echo "$p" | tr -dc '0-9')
  if [ "$Number" ]; then
    Numbers+=($Number)
  fi
done < ../Src/Version.h

VersionString=${Numbers[1]}.${Numbers[2]}-${Numbers[3]}
echo $VersionString

PackageName="tacittexview_${VersionString}"
echo $PackageName
rm -rf ${PackageName}
mkdir ${PackageName}
cp -rf deb_template/* ${PackageName}/

sed -i "s/<TacitVersionString>/${VersionString}/" "${PackageName}/DEBIAN/control"

cp ../TacitTexView "${PackageName}/usr/bin/tacittexview"

# png txt ttf 
cp -f ../Data/*.png "${PackageName}/usr/share/tacittexview/Data/"
cp -f ../Data/*.txt "${PackageName}/usr/share/tacittexview/Data/"
cp -f ../Data/*.ttf "${PackageName}/usr/share/tacittexview/Data/"
fakeroot dpkg-deb --build ${PackageName}

#sleep 2s
#sudo dpkg -r tacittexview
#sleep 2s
#sudo dpkg -i tacittexview_1.0-7.deb


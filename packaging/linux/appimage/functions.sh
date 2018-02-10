# This file is supposed to be sourced by each Recipe
# that wants to use the functions contained herein
# like so:
# wget -q https://github.com/probonopd/AppImages/raw/master/functions.sh -O ./functions.sh
# . ./functions.sh

# RECIPE=$(realpath "$0")

# Options for apt-get to use local files rather than the system ones
OPTIONS="-o Debug::NoLocking=1
-o APT::Cache-Limit=125829120
-o Dir::Etc::sourcelist=./sources.list
-o Dir::State=./tmp
-o Dir::Cache=./tmp
-o Dir::State::status=./status
-o Dir::Etc::sourceparts=-
-o APT::Get::List-Cleanup=0
-o APT::Get::AllowUnauthenticated=1
-o Debug::pkgProblemResolver=true
-o Debug::pkgDepCache::AutoInstall=true
-o APT::Install-Recommends=0
-o APT::Install-Suggests=0
"

# Either get the file from remote or from a static place.
# critical for builds without network access like in Open Build Service
cat_file_from_url()
{
  cat_excludelist="wget -q $1 -O -"
  [ -e "$STATIC_FILES/${1##*/}" ] && cat_excludelist="cat $STATIC_FILES/${1##*/}"
  $cat_excludelist
}

git_pull_rebase_helper()
{
  git reset --hard HEAD
  git pull
}

# Patch /usr to ././ in ./usr
# to make the contents of usr/ relocateable
# (this requires us to cd ./usr before running the application; AppRun does that)
patch_usr()
{
  find usr/ -type f -executable -exec sed -i -e "s|/usr|././|g" {} \;
}

# Download AppRun and make it executable
get_apprun()
{
  # wget -c https://github.com/probonopd/AppImageKit/releases/download/5/AppRun -O ./AppRun # 64-bit
  wget -c https://github.com/probonopd/AppImageKit/releases/download/continuous/AppRun-x86_64 -O AppRun # 64-bit
  chmod a+x AppRun
}

# Copy the library dependencies of all exectuable files in the current directory
# (it can be beneficial to run this multiple times)
copy_deps()
{
  PWD=$(readlink -f .)
  FILES=$(find . -type f -executable -or -name *.so.* -or -name *.so | sort | uniq )
  for FILE in $FILES ; do
    ldd "${FILE}" | grep "=>" | awk '{print $3}' | xargs -I '{}' echo '{}' >> DEPSFILE
  done
  DEPS=$(cat DEPSFILE | sort | uniq)
  for FILE in $DEPS ; do
    if [ -e $FILE ] && [[ $(readlink -f $FILE)/ != $PWD/* ]] ; then
      cp -v --parents -rfL $FILE ./ || true
    fi
  done
  rm -f DEPSFILE
}

# Move ./lib/ tree to ./usr/lib/
move_lib()
{
  mkdir -p ./usr/lib ./lib && find ./lib/ -exec cp -v --parents -rfL {} ./usr/ \; && rm -rf ./lib
  mkdir -p ./usr/lib ./lib64 && find ./lib64/ -exec cp -v --parents -rfL {} ./usr/ \; && rm -rf ./lib64
}

# Delete blacklisted files
delete_blacklisted()
{
  BLACKLISTED_FILES=$(cat_file_from_url https://github.com/probonopd/AppImages/raw/master/excludelist | sed 's|#.*||g')
  echo $BLACKLISTED_FILES
  for FILE in $BLACKLISTED_FILES ; do
    FOUND=$(find . -xtype f -name "${FILE}" 2>/dev/null)
    if [ ! -z "$FOUND" ] ; then
      echo "Deleting blacklisted ${FOUND}"
      rm -f "${FOUND}"
    fi
  done

  # Do not bundle developer stuff
  rm -rf usr/include || true
  rm -rf usr/lib/cmake || true
  rm -rf usr/lib/pkgconfig || true
  find . -name '*.la' | xargs -i rm {}
}

# Echo highest glibc version needed by the executable files in the current directory
glibc_needed()
{
  find . -name *.so -or -name *.so.* -or -type f -executable  -exec readelf -s '{}' 2>/dev/null \; | sed -n 's/.*@GLIBC_//p'| awk '{print $1}' | sort --version-sort | tail -n 1
}
# Add desktop integration
# Usage: get_desktopintegration name_of_desktop_file_and_exectuable
get_desktopintegration()
{
  REALBIN=$(grep -o "^Exec=.*" *.desktop | sed -e 's|Exec=||g' | cut -d " " -f 1 | head -n 1)
  cat_file_from_url https://raw.githubusercontent.com/probonopd/AppImageKit/master/desktopintegration > ./usr/bin/$REALBIN.wrapper
  chmod a+x ./usr/bin/$REALBIN.wrapper

  sed -i -e "s|^Exec=$REALBIN|Exec=$REALBIN.wrapper|g" $1.desktop
}

# Generate AppImage; this expects $ARCH, $APP and $VERSION to be set
generate_appimage()
{
  # if [[ "$RECIPE" == *ecipe ]] ; then
  #   echo "#!/bin/bash -ex" > ./$APP.AppDir/Recipe
  #   echo "# This recipe was used to generate this AppImage." >> ./$APP.AppDir/Recipe
  #   echo "# See http://appimage.org for more information." >> ./$APP.AppDir/Recipe
  #   echo "" >> ./$APP.AppDir/Recipe
  #   cat $RECIPE >> ./$APP.AppDir/Recipe
  # fi
  #
  # Detect the architecture of what we are packaging.
  # The main binary could be a script, so let's use a .so library
  BIN=$(find . -name *.so* -type f | head -n 1)
  INFO=$(file "$BIN")
  if [ -z $ARCH ] ; then
    if [[ $INFO == *"x86-64"* ]] ; then
      ARCH=x86_64
    elif [[ $INFO == *"i686"* ]] ; then
      ARCH=i686
    elif [[ $INFO == *"armv6l"* ]] ; then
      ARCH=armhf
    else
      echo "Could not automatically detect the architecture."
      echo "Please set the \$ARCH environment variable."
     exit 1
    fi
  fi
  wget -c "https://github.com/probonopd/AppImageKit/releases/download/6/AppImageAssistant_6-x86_64.AppImage" -O  AppImageAssistant # (64-bit)
  chmod a+x ./AppImageAssistant
  mkdir -p ../out || true
  rm ../out/$APP"-"$VERSION"-x86_64.AppImage" 2>/dev/null || true
  GLIBC_NEEDED=${GLIBC_NEEDED:=$(glibc_needed)}
  ./AppImageAssistant ./$APP.AppDir/ ../out/$APP"-"$VERSION".glibc"$GLIBC_NEEDED"-"$ARCH".AppImage"
}

# Generate AppImage type 2
generate_type2_appimage()
{
  # Get the ID of the last successful build on Travis CI
  # ID=$(wget -q https://api.travis-ci.org/repos/probonopd/appimagetool/builds -O - | head -n 1 | sed -e 's|}|\n|g' | grep '"result":0' | head -n 1 | sed -e 's|,|\n|g' | grep '"id"' | cut -d ":" -f 2)
  # Get the transfer.sh URL from the logfile of the last successful build on Travis CI
  # Only Travis knows why build ID and job ID don't match and why the above doesn't give both...
  # URL=$(wget -q "https://s3.amazonaws.com/archive.travis-ci.org/jobs/$((ID+1))/log.txt" -O - | grep "https://transfer.sh/.*/appimagetool" | tail -n 1 | sed -e 's|\r||g')
  # if [ -z "$URL" ] ; then
  #   URL=$(wget -q "https://s3.amazonaws.com/archive.travis-ci.org/jobs/$((ID+2))/log.txt" -O - | grep "https://transfer.sh/.*/appimagetool" | tail -n 1 | sed -e 's|\r||g')
  # fi
  URL="https://github.com/probonopd/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
  wget -c "$URL" -O appimagetool
  chmod a+x ./appimagetool
  set +x
  if ( [ ! -z "$KEY" ] ) && ( ! -z "$TRAVIS" ) ; then
    wget https://github.com/probonopd/AppImageKit/files/584665/data.zip -O data.tar.gz.gpg
    ( set +x ; echo $KEY | gpg2 --batch --passphrase-fd 0 --no-tty --skip-verify --output data.tar.gz --decrypt data.tar.gz.gpg )
    tar xf data.tar.gz
    sudo chown -R $USER .gnu*
    mv $HOME/.gnu* $HOME/.gnu_old ; mv .gnu* $HOME/
    GLIBC_NEEDED=${GLIBC_NEEDED:=$(glibc_needed)}
    VERSION=$VERSION.glibc$GLIBC_NEEDED ./appimagetool -n -s --bintray-user $BINTRAY_USER --bintray-repo $BINTRAY_REPO -v ./$APP.AppDir/
  else
    GLIBC_NEEDED=${GLIBC_NEEDED:=$(glibc_needed)}
    VERSION=$VERSION.glibc$GLIBC_NEEDED ./appimagetool -n --bintray-user $BINTRAY_USER --bintray-repo $BINTRAY_REPO -v ./$APP.AppDir/
  fi
  set -x
  mkdir -p ../out/ || true
  mv *.AppImage* ../out/
}

# Generate status file for use by apt-get; assuming that the recipe uses no newer
# ingredients than what would require more recent dependencies than what we assume
# to be part of the base system
generate_status()
{
  mkdir -p ./tmp/archives/
  mkdir -p ./tmp/lists/partial
  touch tmp/pkgcache.bin tmp/srcpkgcache.bin
  wget -q -c "https://github.com/probonopd/AppImages/raw/master/excludedeblist"
  rm status 2>/dev/null || true
  for PACKAGE in $(cat excludedeblist | cut -d "#" -f 1) ; do
    printf "Package: $PACKAGE\nStatus: install ok installed\nArchitecture: all\nVersion: 9:999.999.999\n\n" >> status
  done
}

# Find the desktop file and copy it to the AppDir
get_desktop()
{
   find usr/share/applications -iname "*${LOWERAPP}.desktop" -exec cp {} . \; || true
}

fix_desktop() {
    # fix trailing semicolons
    for key in Actions Categories Implements Keywords MimeType NotShowIn OnlyShowIn; do
      sed -i '/'"$key"'.*[^;]$/s/$/;/' $1
    done
}

# Find the icon file and copy it to the AppDir
get_icon()
{
  find ./usr/share/pixmaps/$LOWERAPP.png -exec cp {} . \; 2>/dev/null || true
  find ./usr/share/icons -path *64* -name $LOWERAPP.png -exec cp {} . \; 2>/dev/null || true
  find ./usr/share/icons -path *128* -name $LOWERAPP.png -exec cp {} . \; 2>/dev/null || true
  find ./usr/share/icons -path *512* -name $LOWERAPP.png -exec cp {} . \; 2>/dev/null || true
  find ./usr/share/icons -path *256* -name $LOWERAPP.png -exec cp {} . \; 2>/dev/null || true
  ls -lh $LOWERAPP.png || true
}

# Find out the version
get_version()
{
  THEDEB=$(find ../*.deb -name $LOWERAPP"_*" | head -n 1)
  if [ -z "$THEDEB" ] ; then
    echo "Version could not be determined from the .deb; you need to determine it manually"
  fi
  VERSION=$(echo $THEDEB | cut -d "~" -f 1 | cut -d "_" -f 2 | cut -d "-" -f 1 | sed -e 's|1%3a||g' | sed -e 's|+dfsg||g' )
  echo $VERSION
}

# transfer.sh
transfer() { if [ $# -eq 0 ]; then echo "No arguments specified. Usage:\necho transfer /tmp/test.md\ncat /tmp/test.md | transfer test.md"; return 1; fi
tmpfile=$( mktemp -t transferXXX ); if tty -s; then basefile=$(basename "$1" | sed -e 's/[^a-zA-Z0-9._-]/-/g'); curl --progress-bar --upload-file "$1" "https://transfer.sh/$basefile" >> $tmpfile; else curl --progress-bar --upload-file "-" "https://transfer.sh/$1" >> $tmpfile ; fi; cat $tmpfile; rm -f $tmpfile; }

# Patch binary files; fill with padding if replacement is shorter than original
# http://everydaywithlinux.blogspot.de/2012/11/patch-strings-in-binary-files-with-sed.html
# Example: patch_strings_in_file foo "/usr/local/lib/foo" "/usr/lib/foo"
patch_strings_in_file() {
    local FILE="$1"
    local PATTERN="$2"
    local REPLACEMENT="$3"
    # Find all unique strings in FILE that contain the pattern
    STRINGS=$(strings ${FILE} | grep ${PATTERN} | sort -u -r)
    if [ "${STRINGS}" != "" ] ; then
        echo "File '${FILE}' contain strings with '${PATTERN}' in them:"
        for OLD_STRING in ${STRINGS} ; do
            # Create the new string with a simple bash-replacement
            NEW_STRING=${OLD_STRING//${PATTERN}/${REPLACEMENT}}
            # Create null terminated ASCII HEX representations of the strings
            OLD_STRING_HEX="$(echo -n ${OLD_STRING} | xxd -g 0 -u -ps -c 256)00"
            NEW_STRING_HEX="$(echo -n ${NEW_STRING} | xxd -g 0 -u -ps -c 256)00"
            if [ ${#NEW_STRING_HEX} -le ${#OLD_STRING_HEX} ] ; then
                # Pad the replacement string with null terminations so the
                # length matches the original string
                while [ ${#NEW_STRING_HEX} -lt ${#OLD_STRING_HEX} ] ; do
                    NEW_STRING_HEX="${NEW_STRING_HEX}00"
                done
                # Now, replace every occurrence of OLD_STRING with NEW_STRING
                echo -n "Replacing ${OLD_STRING} with ${NEW_STRING}... "
                hexdump -ve '1/1 "%.2X"' ${FILE} | \
                sed "s/${OLD_STRING_HEX}/${NEW_STRING_HEX}/g" | \
                xxd -r -p > ${FILE}.tmp
                chmod --reference ${FILE} ${FILE}.tmp
                mv ${FILE}.tmp ${FILE}
                echo "Done!"
            else
                echo "New string '${NEW_STRING}' is longer than old" \
                     "string '${OLD_STRING}'. Skipping."
            fi
        done
    fi
}

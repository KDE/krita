#!/usr/bin/env bash

if test -z $BUILDROOT; then
    echo "ERROR: BUILDROOT env not set, exiting!"
    echo "\t Must point to the root of the buildfiles as stated in 3rdparty Readme"
    exit
fi

DEPBUILD_X86_64_DIR="i.x86_64"
DEPBUILD_ARM64_DIR="i.arm64"
DEPBUILD_FATBIN_DIR="i.universal"

# prototype for universal building for apps
build_x86_64 () {
	echo "building builddeps_x86"
	# first set terminal to arch
	if [[ ${#@} > 0 ]]; then
		for pkg in ${@:1:${#@}}; do
			env /usr/bin/arch -x86_64 /bin/zsh -c "${BUILDROOT}/krita/packaging/macos/osxbuild.sh builddeps --dirty ${pkg}"
		done
	else
		env /usr/bin/arch -x86_64 /bin/zsh -c "${BUILDROOT}/krita/packaging/macos/osxbuild.sh builddeps"
	fi

	rsync -aq i/ ${DEPBUILD_X86_64_DIR}
}


build_arm64 () {
	echo "building builddeps_arm64"

	${BUILDROOT}/krita/packaging/macos/osxbuild.sh builddeps "${@:1}"

	rsync -aq i/ ${DEPBUILD_ARM64_DIR}
}

consolidate_universal_binaries () {
	for f in "${@}"; do
		# Abort if file is not a Mach-O executable file
		if [[ -z $(file ${f} | grep "Mach-O") ]]; then
			continue
		fi
		# echo "${BUILDROOT}/${DEPBUILD_X86_64_DIR}/${f##*test-i/}"
		LIPO_OUTPUT=$(lipo -info ${f} | grep Non-fat 2> /dev/null)
		if [[ -n ${LIPO_OUTPUT} ]]; then
			if [[ -f "${BUILDROOT}/${DEPBUILD_X86_64_DIR}/${f##*${DEPBUILD_FATBIN_DIR}/}" ]]; then
				echo "creating universal binary -- ${f##*${DEPBUILD_FATBIN_DIR}/}"
				lipo -create "${f}" "${BUILDROOT}/${DEPBUILD_X86_64_DIR}/${f##*${DEPBUILD_FATBIN_DIR}/}" -output "${f}" 2> /dev/null
			else
				echo "removing... ${f}"
				rm "${f}"
			fi
		fi
	done

}

prebuild_cleanup() {
	# asume if an argument is given is a package name, do not erase install dir
	if [[ ${#@} > 0 ]]; then
		rsync -aq i/ i.temp
	fi
	rm -rf ${DEPBUILD_FATBIN_DIR} ${DEPBUILD_X86_64_DIR} ${DEPBUILD_ARM64_DIR}
}

postbuild_cleanup() {
	rsync -rlptgoq --ignore-existing ${DEPBUILD_X86_64_DIR}/ ${DEPBUILD_FATBIN_DIR}
	rm -rf i
	rsync -aq ${DEPBUILD_FATBIN_DIR}/ i
}

# rsync -aq i/ i.temp
# rm -rf ${DEPBUILD_FATBIN_DIR} ${DEPBUILD_X86_64_DIR} ${DEPBUILD_ARM64_DIR}

prebuild_cleanup ${@:1}
build_x86_64 ${@:1}

build_arm64 "${@:1}"

rsync -aq ${DEPBUILD_ARM64_DIR}/ ${DEPBUILD_FATBIN_DIR}
consolidate_universal_binaries $(find "${BUILDROOT}/${DEPBUILD_FATBIN_DIR}" -type f)
# consolidate_universal_binaries $(find "${BUILDROOT}/${DEPBUILD_FATBIN_DIR}" -type f -not -perm +111 -name "*dylib")
# consolidate_universal_binaries $(find "${BUILDROOT}/${DEPBUILD_FATBIN_DIR}" -type f -name "*.a")
# consolidate_universal_binaries $(find "${BUILDROOT}/${DEPBUILD_FATBIN_DIR}" -type f -perm +111 -not -name "*.sh" -or -name "*.dylib" -or -name "*.a")

postbuild_cleanup

# rm -rf i
# rsync -aq ${DEPBUILD_FATBIN_DIR}/ i
# mv i.temp i


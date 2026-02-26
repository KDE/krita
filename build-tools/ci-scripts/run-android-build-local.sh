#!/usr/bin/env bash
# Full Android build (Option 2): build Krita for Android then package the APK.
# Run from the Krita repo root. Requires Android SDK and NDK.
#
# Prereqs:
#   - krita-deps-management and krita-deps-management/ci-utilities cloned (see README.android.md)
#   - Android SDK (set ANDROID_HOME or KDECI_ANDROID_SDK_ROOT)
#   - Android NDK (set ANDROID_NDK_HOME or KDECI_ANDROID_NDK_ROOT)
#   - Python 3 with venv and pip
#
# Note: The reference build environment is Linux (GitLab CI). On macOS, CMake may
# fail during OBJC compiler detection when cross-compiling for Android. If that
# happens, build on Linux (e.g. CI, or a Linux VM with the same script) to get the APK.
set -e

KRITA_SRC="$(cd -P "$(dirname "$0")/../.." && pwd)"
cd "$KRITA_SRC"

# --- Config (override with env) ---
DEPS_BRANCH="${DEPS_BRANCH_NAME_ANDROID:-master}"
ANDROID_ABI="${KDECI_ANDROID_ABI:-arm64-v8a}"
WORKDIR="${KDECI_WORKDIR_PATH:-$(pwd)/android-build-wd}"
CACHE_PATH="${KDECI_CACHE_PATH:-$(pwd)/android-build-cache}"
PACKAGE_PROJECT="${KDECI_PACKAGE_PROJECT:-teams/ci-artifacts/krita-android-${ANDROID_ABI}}"
GITLAB_SERVER="${KDECI_GITLAB_SERVER:-https://invent.kde.org/}"
CURRENT_BRANCH="$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo master)"

# --- Resolve Android SDK / NDK ---
if [ -z "$KDECI_ANDROID_SDK_ROOT" ]; then
  if [ -n "$ANDROID_HOME" ]; then
    KDECI_ANDROID_SDK_ROOT="$ANDROID_HOME"
  elif [ -d "$HOME/Library/Android/sdk" ]; then
    KDECI_ANDROID_SDK_ROOT="$HOME/Library/Android/sdk"
  elif [ -d "$HOME/Android/Sdk" ]; then
    KDECI_ANDROID_SDK_ROOT="$HOME/Android/Sdk"
  else
    echo "## ERROR: Set ANDROID_HOME or KDECI_ANDROID_SDK_ROOT to your Android SDK root."
    echo "## On macOS with Android Studio: \$HOME/Library/Android/sdk"
    echo "## On Linux with Android Studio: \$HOME/Android/Sdk"
    exit 1
  fi
fi
if [ -z "$KDECI_ANDROID_NDK_ROOT" ]; then
  if [ -n "$ANDROID_NDK_HOME" ]; then
    KDECI_ANDROID_NDK_ROOT="$ANDROID_NDK_HOME"
  elif [ -d "$KDECI_ANDROID_SDK_ROOT/ndk" ]; then
    KDECI_ANDROID_NDK_ROOT="$(ls -d "$KDECI_ANDROID_SDK_ROOT"/ndk/*/ 2>/dev/null | sort -V | tail -n1)"
    KDECI_ANDROID_NDK_ROOT="${KDECI_ANDROID_NDK_ROOT%/}"
  fi
fi
if [ -z "$KDECI_ANDROID_NDK_ROOT" ] || [ ! -d "$KDECI_ANDROID_NDK_ROOT" ]; then
  echo "## ERROR: Android NDK not found. Set ANDROID_NDK_HOME or KDECI_ANDROID_NDK_ROOT."
  echo "## Install NDK via Android Studio: SDK Manager -> SDK Tools -> NDK (Side by side)."
  exit 1
fi

# --- Use JDK 17 or 21 with compiler for Gradle (AGP 8.12 / Gradle 8.13 do not support Java 25) ---
_jdk_ok() {
  [ -n "$1" ] && [ -x "$1/bin/java" ] && [ -x "$1/bin/javac" ] && "$1/bin/java" -version 2>/dev/null | grep -qE "version \"(1[78]|2[01])\."
}
if ! _jdk_ok "$JAVA_HOME"; then
  JAVA_HOME=""
  for jdk in java-21-openjdk java-17-openjdk java-21-openjdk-devel java-17-openjdk-devel java-21 java-17; do
    for base in /usr/lib/jvm /usr/lib64/jvm; do
      if [ -d "$base/$jdk" ] && [ -x "$base/$jdk/bin/javac" ] && [ -x "$base/$jdk/bin/java" ]; then
        if "$base/$jdk/bin/java" -version 2>/dev/null | grep -qE "version \"(1[78]|2[01])\."; then
          export JAVA_HOME="$base/$jdk"
          break 2
        fi
      fi
    done
  done
fi
if [ -n "$JAVA_HOME" ]; then
  echo "## Using JAVA_HOME=$JAVA_HOME for Gradle (with javac)"
  "$JAVA_HOME/bin/java" -version 2>&1 || true
else
  echo "## ERROR: Gradle (APK step) needs a full JDK 17 or 21 with javac. Install one and set JAVA_HOME, then re-run:"
  echo "##   Fedora/RHEL: sudo dnf install java-21-openjdk-devel"
  echo "##   export JAVA_HOME=/usr/lib/jvm/java-21-openjdk   # or /usr/lib64/jvm/java-21-openjdk"
  exit 1
fi
# If create-apk fails with "Android platform 'android-35' does not exist", install it: SDK Manager -> SDK Platform -> Android 15.0 (API 35).

echo "## Krita Android build (Option 2)"
echo "## Source: $KRITA_SRC"
echo "## Workdir: $WORKDIR"
echo "## Cache: $CACHE_PATH"
echo "## ABI: $ANDROID_ABI"
echo "## SDK: $KDECI_ANDROID_SDK_ROOT"
echo "## NDK: $KDECI_ANDROID_NDK_ROOT"
echo "## Branch: $CURRENT_BRANCH"

# --- Ensure deps-management and ci-utilities ---
if [ ! -d "krita-deps-management" ]; then
  echo "## Cloning krita-deps-management..."
  git clone https://invent.kde.org/packaging/krita-deps-management.git -b "$DEPS_BRANCH" --depth=1
fi
if [ ! -d "krita-deps-management/ci-utilities" ]; then
  echo "## Cloning krita-ci-utilities into krita-deps-management/ci-utilities..."
  git clone https://invent.kde.org/packaging/krita-ci-utilities.git krita-deps-management/ci-utilities --depth=1
fi

# --- Python venv for CI scripts ---
if [ ! -d "venv" ]; then
  echo "## Creating venv and installing krita-deps-management requirements..."
  python3 -m venv venv
  ./venv/bin/pip install -r krita-deps-management/requirements.txt -q
fi
# Use venv Python for the scripts that need yaml etc.; leave VIRTUAL_ENV unset so cmake/ninja use system PATH
PYTHON="$(pwd)/venv/bin/python3"

# --- Generate .kde-ci.yml ---
echo "## Generating .kde-ci.yml..."
$PYTHON krita-deps-management/tools/replace-branch-in-seed-file.py \
  krita-deps-management/latest/krita-deps.yml \
  -p -o branch-corrected-deps.yml -d "$DEPS_BRANCH"
$PYTHON krita-deps-management/tools/generate-deps-file.py \
  -s branch-corrected-deps.yml -o .kde-ci.yml

# --- Prepare env for run-ci-build and build-android-package ---
# run-ci-build uses baseWorkDirectoryPath = KDECI_WORKDIR_PATH / "krita", so _build and _install go under $WORKDIR/krita/.
export KDECI_ANDROID_ABI="$ANDROID_ABI"
export KDECI_ANDROID_SDK_ROOT
export KDECI_ANDROID_NDK_ROOT
export KDECI_WORKDIR_PATH="$WORKDIR"
mkdir -p "$WORKDIR"

export KDECI_REPO_METADATA_PATH="$(pwd)/krita-deps-management/repo-metadata"
export KDECI_PACKAGE_PROJECT="$PACKAGE_PROJECT"
export KDECI_GITLAB_SERVER="$GITLAB_SERVER"
export KDECI_CACHE_PATH="$CACHE_PATH"
export KDECI_SKIP_ECM_ANDROID_TOOLCHAIN=True
TOOLCHAIN_FILE="$(pwd)/krita-deps-management/tools/android-toolchain-krita.cmake"
export KDECI_EXTRA_CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE -DHIDE_SAFE_ASSERTS=OFF -DBUILD_TESTING=ON"

mkdir -p "$CACHE_PATH"

# --- 1) Full Krita Android build (deps + build + install) ---
echo "## Running run-ci-build.py (this will download deps and build; may take a long time)..."
$PYTHON krita-deps-management/ci-utilities/run-ci-build.py \
  --project krita \
  --branch "$CURRENT_BRANCH" \
  --platform "Android/${ANDROID_ABI}/Qt5/Shared" \
  --only-build \
  --skip-publishing \
  2>&1 | tee build-krita.log

# --- 2) Package APK (build-android-package.py uses KDECI_WORKDIR_PATH/krita/_install) ---
echo "## Running build-android-package.py..."
$PYTHON build-tools/ci-scripts/build-android-package.py 2>&1 | tee build-apk.log

# --- Copy APK to repo _packaging for convenience ---
if [ -d "$WORKDIR/krita/_packaging" ]; then
  mkdir -p _packaging
  cp -f "$WORKDIR"/krita/_packaging/*.apk _packaging/ 2>/dev/null || true
  echo "## APK(s) in _packaging:"
  ls -la _packaging/
fi
echo "## Done. APK also under: $WORKDIR/krita/_packaging/"

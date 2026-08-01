#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: CC0-1.0

# The name of catalog we create (without the.pot extension)
# Required by scripty
FILENAME="krita-android"

# relative path to the Android resource folder
ANDROID_RES_DIR="packaging/android/apk/res"

# Called by scripty for message extraction
# First parameter will be the path of the pot file we have to create, includes $FILENAME
function export_pot_file
{
    mkdir outdir
    ANSI_COLORS_DISABLED=1 a2po export --android $ANDROID_RES_DIR --gettext outdir
    mv outdir/template.pot $1
    rm -rf outdir
    rm -f rc.cpp
}

# Called by scripty for merging messages
# First parameter will be a path that will contain several .po files with the format LANG.po
function import_po_files
{
    podir=$1
    # Android doesn't support languages with an @
    find "$podir" -type f -name "*@*.po" -delete
    # drop obsolete messages, as Babel cannot parse them -- see:
    # https://github.com/python-babel/babel/issues/206
    # https://github.com/python-babel/babel/issues/566
    find "$podir" -name '*.po' -exec msgattrib --no-obsolete -o {} {} \;
    ANSI_COLORS_DISABLED=1 a2po import --ignore-fuzzy --android $ANDROID_RES_DIR --gettext $podir
}

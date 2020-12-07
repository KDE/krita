#!/bin/bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

git checkout HEAD .
git clean -f
git reset
git cherry-pick --continue

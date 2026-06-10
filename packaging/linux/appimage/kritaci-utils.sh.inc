#!/bin/bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

val="${KRITACI_VERBOSE_PACKAGING:-0}"
val="$(printf '%s' "$val" | tr '[:upper:]' '[:lower:]')"
case "$val" in
  true|1|t|y|yes) useVerbosePackagingLog=1 ;;
  *)              useVerbosePackagingLog=0 ;;
esac

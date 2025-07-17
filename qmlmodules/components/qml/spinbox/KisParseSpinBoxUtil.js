/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
/**
  This function tries to truncate a float value, so that
  it will always show the minimum amount of decimals behind
  the dot.

  TODO: This isn't locale friendly.
  */
function truncateValue(value, decimals)
{
    if (isNaN(parseFloat(value))) return 0;
    const reg = new RegExp("^-?\\d+(?:\\.\\d{0," + decimals + "})?", "g");
    const a = value.toString().match(reg)[0];
    const dot = a.indexOf(".");
    if (dot === -1) {
        if (decimals < 1) {
            return a;
        }
        return a + "." + "0".repeat(decimals);
    }
    if (decimals < 1) {
        return a.slice(0, dot);
    }
    const b = decimals - (a.length - dot) + 1;
    return b > 0 ? (a + "0".repeat(b)) : a;
}

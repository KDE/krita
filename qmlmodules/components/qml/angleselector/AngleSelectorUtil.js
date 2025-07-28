/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
function closestCoterminalAngleInRange(angle, from, to, errorCallback)
{
    let hasCoterminalAngleInRange = true;

    if (angle < from) {
        const d = from - angle;
        const cycles = Math.floor(d / 360) + 1;
        angle += cycles * 360;
        if (angle > to) {
            hasCoterminalAngleInRange = false;
            angle = from;
        }
    } else if (angle > to) {
        const d = angle - to;
        const cycles = Math.floor(d / 360) + 1;
        angle -= cycles * 360;
        if (angle < from) {
            hasCoterminalAngleInRange = false;
            angle = to;
        }
    }

    if (!hasCoterminalAngleInRange && errorCallback) {
        errorCallback();
    }
    return angle;
}

function flipAngle(angle, horizontal, vertical)
{
    if (horizontal && vertical) {
        angle += 180;
    } else if (horizontal) {
        let a = angle % 360;
        if (a < 0) {
            a += 360;
        }
        if (a > 270) {
            angle -= 2 * (a - 270);
        } else if (a > 180) {
            angle += 2 * (270 - a);
        } else if (a > 90) {
            angle -= 2 * (a - 90);
        } else {
            angle += 2 * (90 - a);
        }
    } else if (vertical) {
        let a = angle % 360;
        if (a < 0) {
            a += 360;
        }
        if (a > 270) {
            angle += 2 * (360 - a);
        } else if (a > 180) {
            angle -= 2 * (a - 180);
        } else if (a > 90) {
            angle += 2 * (180 - a);
        } else {
            angle -= 2 * a;
        }
    }

    return angle;
}

function flipAngleInRange(angle, from, to, horizontal, vertical, errorCallback)
{
    return closestCoterminalAngleInRange(flipAngle(angle, horizontal, vertical), from, to, errorCallback);
}

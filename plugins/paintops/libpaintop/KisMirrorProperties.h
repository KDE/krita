/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISMIRRORPROPERTIES_H
#define KISMIRRORPROPERTIES_H

struct MirrorProperties
{
    MirrorProperties()
        : horizontalMirror(false),
          verticalMirror(false),
          coordinateSystemFlipped(false) {}

    bool horizontalMirror;
    bool verticalMirror;

    bool coordinateSystemFlipped;

    bool isEmpty() const {
        return !horizontalMirror && !verticalMirror;
    }
};

#endif // KISMIRRORPROPERTIES_H

/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSELECTIONTAGS_H
#define KISSELECTIONTAGS_H


enum SelectionMode {
    PIXEL_SELECTION = 0,
    SHAPE_PROTECTION
};

enum SelectionAction {
    SELECTION_REPLACE = 0,
    SELECTION_ADD,
    SELECTION_SUBTRACT,
    SELECTION_INTERSECT,
    SELECTION_SYMMETRICDIFFERENCE,
    SELECTION_DEFAULT
};

#endif // KISSELECTIONTAGS_H

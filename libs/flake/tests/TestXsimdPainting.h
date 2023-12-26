/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTXSIMDPAINTING_H
#define TESTXSIMDPAINTING_H

#include <simpletest.h>

/**
 * @brief The TestXsimdPainting class
 *
 * This tests whether the algorithms for XSimd and its fallback algorithm are the same.
 */

class TestXsimdPainting : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testKoClipMaskPainting_data();
    void testKoClipMaskPainting();
};

#endif // TESTXSIMDPAINTING_H

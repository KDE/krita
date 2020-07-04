/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_IMAGEPIPE_BRUSH_TEST_H
#define __KIS_IMAGEPIPE_BRUSH_TEST_H

#include <QtTest>

#include <kis_imagepipe_brush.h>

class KisImagePipeBrushTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoading();
    void testChangingBrushes();
    void testSimpleDabApplication();
    void testColoredDab();
    void testColoredDabWash();
    void testTextBrushNoPipes();
    void testTextBrushPiped();

private:
    void checkConsistency(KisImagePipeBrushSP brush);
};

#endif /* __KIS_IMAGEPIPE_BRUSH_TEST_H */

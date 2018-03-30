/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisHandleStyle.h"
#include "kis_painting_tweaks.h"

namespace {
void initDashedStyle(const QColor &baseColor, const QColor &handleFill, KisHandleStyle *style) {
    QPen ants;
    QPen outline;
    KisPaintingTweaks::initAntsPen(&ants, &outline);

    ants.setColor(baseColor);

    style->lineIterations << KisHandleStyle::IterationStyle(outline, Qt::NoBrush);
    style->lineIterations << KisHandleStyle::IterationStyle(ants, Qt::NoBrush);

    QPen handlePen(baseColor);
    handlePen.setWidth(2);
    handlePen.setJoinStyle(Qt::RoundJoin);

    style->handleIterations << KisHandleStyle::IterationStyle(handlePen, handleFill);
}

static const QColor primaryColor(0, 0, 90, 180);
static const QColor secondaryColor(0, 0, 255, 127);
static const QColor gradientFillColor(255, 197, 39);
static const QColor highlightColor(255, 100, 100);
static const QColor highlightOutlineColor(155, 0, 0);
static const QColor selectionColor(164, 227, 243);

}


KisHandleStyle &KisHandleStyle::inheritStyle()
{
    static QScopedPointer<KisHandleStyle> style;

    if (!style) {
        style.reset(new KisHandleStyle());
        style->lineIterations << KisHandleStyle::IterationStyle();
        style->handleIterations << KisHandleStyle::IterationStyle();
    }

    return *style;
}

KisHandleStyle &KisHandleStyle::primarySelection()
{
    static QScopedPointer<KisHandleStyle> style;

    if (!style) {
        style.reset(new KisHandleStyle());
        initDashedStyle(primaryColor, Qt::white, style.data());
    }

    return *style;
}

KisHandleStyle &KisHandleStyle::secondarySelection()
{
    static QScopedPointer<KisHandleStyle> style;

    if (!style) {
        style.reset(new KisHandleStyle());
        initDashedStyle(secondaryColor, Qt::white, style.data());
    }

    return *style;
}

KisHandleStyle &KisHandleStyle::gradientHandles()
{
    static QScopedPointer<KisHandleStyle> style;

    if (!style) {
        style.reset(new KisHandleStyle());
        initDashedStyle(primaryColor, gradientFillColor, style.data());
    }

    return *style;
}

KisHandleStyle &KisHandleStyle::gradientArrows()
{
    return primarySelection();
}


KisHandleStyle &KisHandleStyle::highlightedPrimaryHandles()
{
    static QScopedPointer<KisHandleStyle> style;

    if (!style) {
        style.reset(new KisHandleStyle());
        initDashedStyle(highlightOutlineColor, highlightColor, style.data());
    }

    return *style;
}

KisHandleStyle &KisHandleStyle::highlightedPrimaryHandlesWithSolidOutline()
{
    static QScopedPointer<KisHandleStyle> style;

    if (!style) {
        style.reset(new KisHandleStyle());
        style->handleIterations << KisHandleStyle::IterationStyle(highlightOutlineColor, highlightColor);
        style->lineIterations << KisHandleStyle::IterationStyle(highlightOutlineColor, Qt::NoBrush);
    }

    return *style;
}

KisHandleStyle &KisHandleStyle::partiallyHighlightedPrimaryHandles()
{
    static QScopedPointer<KisHandleStyle> style;

    if (!style) {
        style.reset(new KisHandleStyle());
        initDashedStyle(highlightOutlineColor, selectionColor, style.data());
    }

    return *style;
}

KisHandleStyle &KisHandleStyle::selectedPrimaryHandles()
{
    static QScopedPointer<KisHandleStyle> style;

    if (!style) {
        style.reset(new KisHandleStyle());
        initDashedStyle(primaryColor, selectionColor, style.data());
    }

    return *style;
}


/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann <cbo@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOTEXTLAYOUTROOTAREAPROVIDER_H
#define KOTEXTLAYOUTROOTAREAPROVIDER_H

#include "textlayout_export.h"

#include <QSizeF>
#include <QRectF>
#include <QList>

class KoTextLayoutRootArea;
class KoTextDocumentLayout;
class KoTextLayoutObstruction;

/**
 * When laying out text we need an area where upon the text will be placed.
 * A KoTextLayoutRootAreaProvider provides the layout process with such areas
 */
class TEXTLAYOUT_EXPORT KoTextLayoutRootAreaProvider
{
public:
    /// constructor
    explicit KoTextLayoutRootAreaProvider();
    virtual ~KoTextLayoutRootAreaProvider();

    /// Provides an new root area
    virtual KoTextLayoutRootArea *provide(KoTextDocumentLayout *documentLayout) = 0;

    /// Release all root areas that are after the "afterThis" root area
    /// If afterThis == 0 all should be released
    virtual void releaseAllAfter(KoTextLayoutRootArea *afterThis) = 0;

    /// This method allows the provider to do any post processing like
    ///   - painting it
    ///   - changing it's size
    ///   - do other things to other structures (eg resizing the textshape)
    virtual void doPostLayout(KoTextLayoutRootArea *rootArea, bool isNewRootArea) = 0;

    /// Returns a suggested a size for the root area
    virtual QSizeF suggestSize(KoTextLayoutRootArea *rootArea) = 0;

    /// Return a list of obstructions intersecting root area
    virtual QList<KoTextLayoutObstruction *> relevantObstructions(KoTextLayoutRootArea *rootArea) = 0;

};

#endif

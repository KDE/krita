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

#include "kritatextlayout_export.h"

#include <QList>
#include <QString>

class KoTextLayoutRootArea;
class KoTextDocumentLayout;
class KoTextLayoutObstruction;
class QRectF;

/**
 * Represents the contract that a root area requested by the layout system
 * has to respect. For simple layout situations (like a single text shape),
 * it's fine to ignore the contract since pages do not exist.
 */
struct RootAreaConstraint {
    QString masterPageName;
    int visiblePageNumber;
    bool newPageForced;
};

/**
 * When laying out text we need an area where upon the text will be placed.
 * A KoTextLayoutRootAreaProvider provides the layout process with such areas
 */
class KRITATEXTLAYOUT_EXPORT KoTextLayoutRootAreaProvider
{
public:
    /// constructor
    explicit KoTextLayoutRootAreaProvider();
    virtual ~KoTextLayoutRootAreaProvider();

    /**
     * Provides a new root area for the layout
     *
     * @param documentLayout the current document layouter
     * @param constraints the rules the new area has to respect (page style, visible page number...)
     * @param requestedPosition the position of the new area in the text flow
     * @param isNewArea will contain a boolean to tell whether this is a new area or a recycled one
     */
    virtual KoTextLayoutRootArea *provide(KoTextDocumentLayout *documentLayout, const RootAreaConstraint &constraints, int requestedPosition, bool *isNewArea) = 0;

    /// Release all root areas that are after the "afterThis" root area
    /// If afterThis == 0 all should be released
    virtual void releaseAllAfter(KoTextLayoutRootArea *afterThis) = 0;

    /// This method allows the provider to do any post processing like
    ///   - painting it
    ///   - changing it's size
    ///   - do other things to other structures (eg resizing the textshape)
    virtual void doPostLayout(KoTextLayoutRootArea *rootArea, bool isNewRootArea) = 0;

    /// Makes all canvases redraw the shapes maintained by this provider
    ///    use with care - it eats a lot of processing for no real gain
    virtual void updateAll() = 0;

    /// Returns a suggested offset and size for the root area
    virtual QRectF suggestRect(KoTextLayoutRootArea *rootArea) = 0;

    /// Return a list of obstructions intersecting root area
    virtual QList<KoTextLayoutObstruction *> relevantObstructions(KoTextLayoutRootArea *rootArea) = 0;
};

#endif

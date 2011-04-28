/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@kogmbh.com>
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

#ifndef KOTEXTLAYOUTROOTAREA_H
#define KOTEXTLAYOUTROOTAREA_H

#include "kotext_export.h"

#include "KoTextLayoutArea.h"

#include <QRectF>

class KoShape;
class KoTextPage;

/**
 * When laying out text it happens in areas that can occupy space of various size.
 */
class KOTEXT_EXPORT KoTextLayoutRootArea : public KoTextLayoutArea
{
public:
    /// constructor
    explicit KoTextLayoutRootArea(KoTextDocumentLayout *documentLayout);
    virtual ~KoTextLayoutRootArea();

    /// Layouts as much as it can
    /// Returns true if it has reached the end of the frame
    virtual bool layout(FrameIterator *cursor);

    /// Sets an associated shape which can be retrieved with associatedShape()
    /// KoTextLayoutRootArea doesn't use it for anything.
    void setAssociatedShape(KoShape *shape);

    /// Retruns the shape set with setAssociatedShape()
    KoShape *associatedShape();

    /**
     * Set the \p page this root area is on.
     *
     * The root-area does NOT take over ownership of the KoTextPage and the user
     * of the root-area is responsible to delete it if not needed any longer.
     */
    void setPage(KoTextPage *textpage);

    /// Returns the page this root area is on.
    KoTextPage *page() const;

    void setDirty();

    bool isDirty() const;

    virtual KoText::Direction parentTextDirection() const;

    void setBottom(qreal b);

private:
    class Private;
    Private * const d;
};

#endif

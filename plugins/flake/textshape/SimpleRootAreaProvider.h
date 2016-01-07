/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
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

#ifndef SIMPLEROOTAREAPROVIDER_H
#define SIMPLEROOTAREAPROVIDER_H

#include "KoTextLayoutRootAreaProvider.h"

class TextShape;
class KoTextShapeData;

class SimpleRootAreaProvider : public KoTextLayoutRootAreaProvider
{
public:
    SimpleRootAreaProvider(KoTextShapeData *data, TextShape *textshape);

    /// reimplemented
    virtual KoTextLayoutRootArea *provide(KoTextDocumentLayout *documentLayout, const RootAreaConstraint &constraints, int requestedPosition, bool *isNewRootArea);

    virtual void releaseAllAfter(KoTextLayoutRootArea *afterThis);

    virtual void doPostLayout(KoTextLayoutRootArea *rootArea, bool isNewRootArea);

    virtual void updateAll();

    virtual QRectF suggestRect(KoTextLayoutRootArea *rootArea);

    virtual QList<KoTextLayoutObstruction *> relevantObstructions(KoTextLayoutRootArea *rootArea);

    TextShape *m_textShape;

    KoTextLayoutRootArea *m_area;
    KoTextShapeData *m_textShapeData;
    bool m_fixAutogrow;
};

#endif

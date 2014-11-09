/* This file is part of the KDE project
 * Copyright (C) 2011 Matus Hanzes <matus.hanzes@ixonos.com>
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

#ifndef ANCHORSTRATEGY_H_
#define ANCHORSTRATEGY_H_

#include "KoShapeAnchor.h"

#include <QRectF>

class KoTextShapeContainerModel;
class KoTextLayoutRootArea;


class AnchorStrategy  : public KoShapeAnchor::PlacementStrategy
{
public:
    AnchorStrategy(KoShapeAnchor *anchor, KoTextLayoutRootArea *rootArea);
    virtual ~AnchorStrategy();

    /**
     * Moves the subject to it's right position.
     *
     * @return true if subject was moved to a new position (or it it couldn't be calculated yet)
     */
    virtual bool moveSubject() = 0;

    virtual void detachFromModel();

    /**
     * Reparent the anchored shape under the rootArea's container this AnchorStrategy acts for
    .*
     * If needed changes the parent KoShapeContainerModel and KoShapeContainer of the anchored shape.
     * It is changed so the anchored shape is now under the rootArea
     */
    virtual void updateContainerModel();

    /// get page rectangle coordinates to which this text anchor is anchored (needed for HPage)
    QRectF pageRect() const;

    /// set page rectangle coordinates to which this text anchor is anchored (needed for HPage)
    void setPageRect(const QRectF &pageRect);

    /// get content rectangle coordinates to which this text anchor is anchored (needed for
    /// HPageContent)
    QRectF pageContentRect() const;

    /// set content rectangle coordinates to which this text anchor is anchored (needed for
    /// HPageContent)
    void setPageContentRect(const QRectF &marginRect);

    /// get paragraph rectangle coordinates to which this text anchor is anchored (needed for
    /// HParagraphContent, HParagraphStartMargin, HParagraphEndMargin, VParagraph)
    QRectF paragraphRect() const;

    /// set paragraph rectangle to which this text anchor is anchored (needed for HParagraphContent,
    /// HParagraphStartMargin, HParagraphEndMargin, VParagraph)
    void setParagraphRect(const QRectF &paragraphRect);

    /// get paragraph rectangle coordinates to which this text anchor is anchored (needed for
    /// HParagraphContent, HParagraphStartMargin, HParagraphEndMargin)
    QRectF paragraphContentRect() const;

    /// set paragraph rectangle to which this text anchor is anchored (needed for HParagraphContent,
    /// HParagraphStartMargin, HParagraphEndMargin)
    void setParagraphContentRect(const QRectF &paragraphContentRect);

    /// get layout environment rectangle @see odf attribute style:flow-with-text
    QRectF layoutEnvironmentRect() const;

     /// set layout environment rect @see odf attribute style:flow-with-text
    void setLayoutEnvironmentRect(const QRectF &layoutEnvironmentRect);

    /// get number of page to which this text anchor is anchored (needed for HOutside, HInside,
    /// HFromInside)
    int pageNumber() const;

    /// set number of page to which this text anchor is anchored (needed for HOutside, HInside,
    /// HFromInside)
    void setPageNumber(int pageNumber);

protected:
    KoShapeAnchor * const m_anchor;
    KoTextLayoutRootArea *m_rootArea;

private:
    KoTextShapeContainerModel *m_model;
    QRectF m_pageRect;
    QRectF m_pageContentRect;
    QRectF m_paragraphRect;
    QRectF m_paragraphContentRect;
    QRectF m_layoutEnvironmentRect;
    int m_pageNumber;
};

#endif /* ANCHORSTRATEGY_H_ */

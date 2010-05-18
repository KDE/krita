/*
 * Copyright (C) 2008-2010 Thomas Zander <zander@kde.org>
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
#ifndef CLIPBOARDPROXYSHAPE_H
#define CLIPBOARDPROXYSHAPE_H

#include <KoShape.h>

#include <QByteArray>

class QPainter;
class KoViewConverter;
#include "KoXmlReaderForward.h"
class KoShapeLoadingContext;
class KoShapeSavingContext;

#define OASIS_MIME "application/vnd.oasis.opendocument.text"

/**
 * The shape selector can show different types of items, the clipboard is one of them.
 * The shape selector is based on Flake shapes and uses shapes for any content it
 * shows.  This allows the shape selector to take any clipboard content which can
 * be shown using a flake shape to actually be visualized in the shape selector.
 * The ClipboardProxyShape takes the content of the clipboard and finds the most
 * appropriate flake shape for it to visualize the content.  As the name 'proxy'
 * indicates, this shape will have a child that shows the actual content (like a picture,
 * or some text).
 */
class ClipboardProxyShape : public KoShape
{
public:
    /*
     * constructs a shape that will show a clipboard item at a specific size,
     * scaling the view on the actual item if needed.
     * Note that we take ownership of the clipboardItem.
     */
    ClipboardProxyShape(KoShape*clipboardItem, const QByteArray &clipboardData);
    virtual ~ClipboardProxyShape();

    /// reimplemented from KoShape
    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    /// reimplemented from KoShape
    virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) { return false; }
    /// reimplemented from KoShape
    virtual void saveOdf(KoShapeSavingContext &) const { }

    /// return the clipboard data that this shape displays
    QByteArray clipboardData() const { return m_clipboardData; }

private:
    KoShape *m_child;
    QByteArray m_clipboardData;
};

#endif

/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef HTMLSAVINGCONTEXT_H
#define HTMLSAVINGCONTEXT_H

#include <QScopedPointer>

class KoXmlWriter;
class KoShape;
class KoImageData;
class QIODevice;
class QString;
class QTransform;
class QImage;

/**
 * @brief The HtmlSavingContext class provides context for saving a flake-based document
 * to html.
 */
class HtmlSavingContext
{
public:
    HtmlSavingContext(QIODevice &shapeDevice);
    virtual ~HtmlSavingContext();
    /// Provides access to the shape writer
    KoXmlWriter &shapeWriter();
private:
    Q_DISABLE_COPY(HtmlSavingContext)
private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // HTMLSAVINGCONTEXT_H

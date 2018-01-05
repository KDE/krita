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

#ifndef HTMLWRITER_H
#define HTMLWRITER_H

#include <QList>
#include <QSizeF>

class KoShapeLayer;
class KoShapeGroup;
class KoShape;
class KoPathShape;
class QIODevice;
class QString;
class HtmlSavingContext;

// Implements writing shapes to HTML
class HtmlWriter
{
public:
    HtmlWriter(const QList<KoShape*> &toplevelShapes);
    virtual ~HtmlWriter();

    bool save(QIODevice &outputDevice);

    QStringList errors() const;
    QStringList warnings() const;

private:

    void saveShapes(const QList<KoShape*> shapes, HtmlSavingContext &savingContext);

    QList<KoShape*> m_toplevelShapes;
    QStringList m_errors;
    QStringList m_warnings;
};

#endif // HTMLWRITER_H

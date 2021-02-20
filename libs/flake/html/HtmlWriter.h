/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

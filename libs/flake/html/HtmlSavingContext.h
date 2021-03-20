/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

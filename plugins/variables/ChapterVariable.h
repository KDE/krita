/* This file is part of the KDE project
 * Copyright (C) 2011 Sebastian Sauer <mail@dipe.org>
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
#ifndef CHAPTERVARIABLE_H
#define CHAPTERVARIABLE_H

#include <QObject>

#include <KoVariable.h>
#include <KoTextShapeData.h>
#include <KoTextPage.h>

class KoShapeSavingContext;

/**
 * This is a KoVariable for chapter variables.
 */
class ChapterVariable : public KoVariable
{
    Q_OBJECT
public:
    ChapterVariable();

    // reimplmented
    QWidget* createOptionsWidget();
    void readProperties(const KoProperties *props);
    void saveOdf(KoShapeSavingContext & context);
    bool loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context);

private Q_SLOTS:
    void formatChanged(int format);
    void levelChanged(int level);

private:
    void resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);

    enum FormatTypes {
        ChapterName,
        ChapterNumber,
        ChapterNumberName,
        ChapterPlainNumber,
        ChapterPlainNumberName
    };

    FormatTypes m_format;
    int m_level;
};

#endif

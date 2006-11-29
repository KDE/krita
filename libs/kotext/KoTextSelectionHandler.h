/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTSELECTIONHANDLER_H
#define KOTEXTSELECTIONHANDLER_H

#include <KoToolSelection.h>

#include <koffice_export.h>

class KoTextShape;
class KoTextShapeData;
class QTextCursor;

/**
 * The public class that is able to manipulate selected text.
 */
class KOTEXT_EXPORT KoTextSelectionHandler : public KoToolSelection {
public:
    KoTextSelectionHandler() {}

    void bold(bool bold);
    void italic(bool italic);
    void underline(bool underline);
    void strikeOut(bool strikeout);

protected:
    friend class KoTextTool;
    void setShape(KoTextShape *shape) { m_textShape = shape; }
    void setShapeData(KoTextShapeData *data) { m_textShapeData = data; }
    void setCaret(QTextCursor *caret) { m_caret = caret; }

private:
    KoTextShape *m_textShape;
    KoTextShapeData *m_textShapeData;
    QTextCursor *m_caret;
};

#endif

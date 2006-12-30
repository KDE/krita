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

// KOffice libs
#include "KoInlineObjectBase.h"
#include <koffice_export.h>

// Qt + kde
#include <QHash>
#include <QString>

class QTextInlineObject;
class QTextDocument;
class KoShape;

/**
 * Base class for in-text variables.
 * A variable is a field inserted into the text and the content is set to a specific value that
 * is used as text.  This class is pretty boring in that it has just a setValue() to alter the
 * text shown; we depend on plugin writers to create more exciting ways to update variables.
 */
class KOTEXT_EXPORT KoVariable : public KoInlineObjectBase {
public:
    /**
     * Constructor.
     */
    KoVariable();
    virtual ~KoVariable() {}

    /**
     * The new value this variable will show.
     * Will be used at the next repaint.
     * @param value the new value this variable shows.
     */
    void setValue(const QString &value);

    /// @return the current value of this variable.
    const QString &value() const { return m_value; }

protected:
    /**
     * This hook is called whenever the variable gets a new position.
     * If this is a type of variable that needs to change its value based on that
     * you should implement this method and act on it.
     */
    virtual void variableMoved(const KoShape *shape, const QTextDocument *document, int posInDocument);

private:
    void updatePosition(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format);
    void resize(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    void paint (QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
            const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

private:
    QString m_value;
    bool m_modified;
    const QTextDocument *m_document;
    int m_lastPositionInDocument;
};

/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#ifndef KOINLINEOBJECTBASE_H
#define KOINLINEOBJECTBASE_H

#include <Qt> // for Q_UNUSED
#include <koffice_export.h>

class QTextDocument;
class QTextCharFormat;
class QTextInlineObject;
class QPaintDevice;
class QPainter;
class QRectF;
class QVariant;

class KoInlineTextObjectManager;

/**
 * Base class for all inline-text-objects.
 * In a KoTextShape you can insert objects that move with the text. They are essentially anchored to a specific
 * position in the text, as one character.
 * @see KoInlineTextObjectManager
 */
class KOTEXT_EXPORT KoInlineObject {
public:
    enum Property {
        DocumentURL,
        PageCount,
        StartPage,
        AuthorName,
        SenderEmail,
        SenderCompany,
        SenderPhoneWork,
        SenderPhonePrivate,
        SenderFax,
        SenderCountry,
        Title,
        Keywords,
        Subject,
        Description,
        SenderPostalCode,
        SenderCity,
        SenderStreet,
        SenderTitle,
        SenderFirstname,
        SenderLastname,
        SenderPosition,
        AuthorInitials,


        KarbonStart = 1000,      ///< Base number for karbon specific values.
        KexiStart = 2000,        ///< Base number for kexi specific values.
        KivioStart = 3000,       ///< Base number for kivio specific values.
        KPlatoStart = 4000,      ///< Base number for kplato specific values.
        KPresenterStart = 5000,  ///< Base number for kpresenter specific values.
        KritaStart = 6000,       ///< Base number for krita specific values.
        KWordStart = 7000,       ///< Base number for kword specific values.
        User = 10000
    };

    /// constructor
    KoInlineObject(bool propertyChangeListener = false)
        : m_manager(0), m_id(-1), m_propertyChangeListener(propertyChangeListener) {}
    virtual ~KoInlineObject() {}

    void setManager(KoInlineTextObjectManager *manager) { m_manager = manager; }
    virtual void initialize() {}


    /**
     * Update position of the inline object.
     * This is called each time the paragraph this inline object is in is re-layouted giving you the opportunity
     * to reposition your object based on the new information.
     * @param document the text document this inline object is operating on.
     * @param object the inline object properties
     * @param posInDocument the character position in the document (param document) this inline object is at.
     * @param format the character format for the inline object.
     */
    virtual void updatePosition(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format) = 0;

    /**
     * Update the size of the inline object.
     * Each time the text is painted, as well as when the paragraph this variable is in, this method
     * is called. You should alter the size of the variable if the content has changed.
     * Altering the size is done by altering the 'object' parameter using QTextInlineObject::setWidth(),
     * QTextInlineObject::setAscent() and QTextInlineObject::setDescent() methods.
     * @param document the text document this inline object is operating on.
     * @param object the inline object properties
     * @param posInDocument the character position in the document (param document) this inline object is at.
     * @param format the character format for the inline object.
     * @param pd the postscript-paintdevice that all text is rendered on. Use this for QFont and related
     *  classes so the inline object can be reused on any paintdevice.
     */
    virtual void resize(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format, QPaintDevice *pd) = 0;

    /**
     * Paint the inline-object-base using the provided painter within the rectangle specified by rect.
     * @param document the text document this inline object is operating on.
     * @param object the inline object properties
     * @param posInDocument the character position in the document (param document) this inline object is at.
     * @param format the character format for the inline object.
     * @param pd the postscript-paintdevice that all text is rendered on. Use this for QFont and related
     *  classes so the inline object can be reused on any paintdevice.
     * @param painter the painting object to paint on.  Note that unline many places in koffice painting
     *    should happen at the position indicated by the rect, not at top-left.
     * @param rect the rectangle inside which the variable can paint itself.  Painting outside the rect
     *    will give varous problems with regards to repainting issues.
     */
    virtual void paint (QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
            const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) = 0;

    virtual void propertyChanged(Property key, const QVariant &value) { Q_UNUSED(key); Q_UNUSED(value); }

    /// return the inline-object Id that is assigned for this object.
    int id() const { return m_id; }
    /// Set the inline-object Id that is assigned for this object by the KoInlineTextObjectManager.
    void setId(int id) { m_id = id; }

    bool propertyChangeListener() const { return m_propertyChangeListener; }

protected:
    KoInlineTextObjectManager *m_manager;

private:
    int m_id;
    const bool m_propertyChangeListener;
};
#endif

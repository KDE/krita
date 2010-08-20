/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
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

#include "kotext_export.h"

#include <QVariant>

class QTextDocument;
class QTextCharFormat;
class QTextInlineObject;
class QPaintDevice;
class QPainter;
class QRectF;

class KoShape;
class KoInlineTextObjectManager;
class KoInlineObjectPrivate;
class KoShapeSavingContext;
class KoTextInlineRdf;
class KoXmlElement;
class KoShapeLoadingContext;

/**
 * Base class for all inline-text-objects.
 *
 * In a TextShape you can insert objects that move with the text.
 * They are essentially anchored to a specific position in the text, as
 * one character.
 *
 * @see KoInlineTextObjectManager
 */
class KOTEXT_EXPORT KoInlineObject
{
public:
    enum Property {
        DocumentURL,
        PageCount,
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
        VariableManagerStart = 8000, ///< Start of numbers reserved for the KoVariableManager
        User = 12000
    };

    /**
     * constructor
     * @param propertyChangeListener if set to true this instance will be notified of changes of properties.
     * @see KoInlineTextObjectManager::setProperty()
     * @see propertyChangeListener()
     */
    explicit KoInlineObject(bool propertyChangeListener = false);
    virtual ~KoInlineObject();

    /**
     * Will be called by the manager when this variable is added.
     * Remember that inheriting classes should not use the manager() in the constructor, since it will be 0
     * @param manager the object manager for this object.
     */
    void setManager(KoInlineTextObjectManager *manager);

    /**
     * Return the object manager set on this inline object.
     */
    KoInlineTextObjectManager *manager();

    /**
     * Just prior to the first time this object will be shown this method will be called.
     * The object plugin should reimplement this to initialize the object after the manager
     * has been set, but before the text has been layouted.
     * The default implementation does nothing.
     */
    virtual void setup() {}

    /**
     * Save this inlineObject as ODF
     * @param context the context for saving.
     */
    virtual void saveOdf(KoShapeSavingContext &context);

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
     * is called. You should alter the size of the object if the content has changed.
     * Altering the size is done by altering the 'object' parameter using QTextInlineObject::setWidth(),
     * QTextInlineObject::setAscent() and QTextInlineObject::setDescent() methods.
     * Note that this method is called while painting; and thus is time sensitive; avoid doing anything time
     * consuming.
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
    virtual void paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
                       const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) = 0;

    /**
     * Overwrite this if you are interrested in propertychanges.
     * @param property the property id that has been changed, one from the Property enum.
     *    You should ignore all properties you don't use as new properties can be added at any time.
     * @param value the new value of the property wrapped in a QVariant.  Properties can be a lot of
     *     different class types. Ints, bools, QStrings etc.
     * example:
     * @code
     *  void KoDateVariable::propertyChanged(Property key, const QVariant &value) {
     *      if(key == KoInlineObject::PageCount)
     *          setValue(QString::number(value.toInt()));
     *  }
     * @endcode
     * @see propertyChangeListener()
     */
    virtual void propertyChanged(Property property, const QVariant &value);

    /// return the inline-object Id that is assigned for this object.
    int id() const;
    /// Set the inline-object Id that is assigned for this object by the KoInlineTextObjectManager.
    void setId(int id);

    /**
     * When true, notify this object of property changes.
     * Each inlineObject can use properties like the PageCount or the document name.
     * Only objects that actually have a need for such information be a listener to avoid unneeded
     * overhead.
     * When this returns true, the propertyChanged() method will be called.
     * @see KoInlineTextObjectManager::setProperty()
     */
    bool propertyChangeListener() const;

    /**
     * An inline object might have some Rdf metadata associated with it
     * in content.xml
     * Ownership of the rdf object is taken by this object, you should not
     * delete it.
     */
    void setInlineRdf(KoTextInlineRdf *rdf);
    /**
     * Get any Rdf which was stored in content.xml for this inline object
     * This object continues to own the object, do not delete it.
     */
    KoTextInlineRdf *inlineRdf() const;

    /**
     * Load a variable from odf.
     *
     * @param element element which represents the shape in odf
     * @param context the KoShapeLoadingContext used for loading
     *
     * @return false if loading failed
     */
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context) =  0;

protected:
    explicit KoInlineObject(KoInlineObjectPrivate &, bool propertyChangeListener = false);

    /**
     * We allow a text document to be shown in more than one shape; which brings up the need to figure out
     * which shape is used for a certain text.
     * @param document the document we are searching in.
     * @param position the position of the character in the text document we want to locate.
     * @return the shape the text is laid-out in.  Or 0 if there is no shape for that text character.
     */
    static KoShape *shapeForPosition(const QTextDocument *document, int position);

    KoInlineObjectPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KoInlineObject)
    friend KOTEXT_EXPORT QDebug operator<<(QDebug, const KoInlineObject *);
};

KOTEXT_EXPORT QDebug operator<<(QDebug dbg, const KoInlineObject *o);

#endif

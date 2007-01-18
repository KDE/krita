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
#ifndef KOINLINEOBJECTMANAGER_H
#define KOINLINEOBJECTMANAGER_H

#include "KoInlineObject.h"
#include "KoInlineObjectFactory.h"
#include "KoVariableManager.h"

#include <kotext_export.h>

// Qt + kde
#include <QHash>
#include <QTextCharFormat>

class KoCanvasBase;
class QAction;

/**
 * A container to register all the inlineTextObjects with.
 * Inserting an inline-object in a QTextDocument should be done via this manager which will
 * insert a placeholder in the text and if you add the KoInlineTextObjectManager to the
 * KoTextDocumentLayout for that specific textDocument, your inline text object will get painted
 * properly.
 */
class KOTEXT_EXPORT KoInlineTextObjectManager : public QObject {
    Q_OBJECT
// TODO, when to delete the inlineObject s
public:
     /// Constructor
    KoInlineTextObjectManager(QObject *parent = 0);

    /**
     * Retrieve a formerly added inline object based on the format.
     * @param format the textCharFormat 
     */
    KoInlineObject *inlineTextObject(const QTextCharFormat &format) const;
    /**
     * Retrieve a formerly added inline object based on the cursor position.
     * @param cursor the cursor which position is used. The anchor is ignored.
     */
    KoInlineObject *inlineTextObject(const QTextCursor &cursor) const;

    /**
     * Insert a new inline object into the manager as well as the document.
     * This method will cause a placeholder to be inserted into the text at cursor position,
     *  possibly replacing a selection.  The object will then be used as an inline
     * character and painted at the specified location in the text.
     * @param cursor the cursor which indicated the document and the position in that document
     *      where the inline object will be inserted.
     * @param object the inline object to insert.
     */
    void insertInlineObject(QTextCursor &cursor, KoInlineObject *object);

    /**
     * Set a property that may have changed which will be forwarded to all registered textObjects.
     * If the key has changed then all registered InlineObject instances that have stated to want
     * updates will get called with the change.
     * The property will be stored to allow it to be retrieved via the intProperty() and friends.
     * @see KoInlineObject::propertyChangeListener()
     */
    void setProperty(KoInlineObject::Property key, QVariant value);
    /// retrieve a propery
    QVariant property(KoInlineObject::Property key) const;
    /// retrieve an int property
    int intProperty(KoInlineObject::Property key) const;
    /// retrieve a bool property
    bool boolProperty(KoInlineObject::Property key) const;
    /// retrieve a string property
    QString stringProperty(KoInlineObject::Property key) const;
    /// remove a property from the store.
    void removeProperty(KoInlineObject::Property key);

    const KoVariableManager *variableManager() const;
    KoVariableManager *variableManager();

    QList<QAction*> createInsertVariableActions(KoCanvasBase *host) const;

signals:
    void propertyChanged(int, const QVariant &variant);

private:
    enum Properties {
        InlineInstanceId = 577297549
    };

    QHash<int, KoInlineObject*> m_objects;
    QList<KoInlineObject*> m_listeners; // holds objects also in m_objects, but which want propertyChanges
    int m_lastObjectId;
    QHash<int, QVariant> m_properties;

    KoVariableManager m_variableManager;
};

#endif

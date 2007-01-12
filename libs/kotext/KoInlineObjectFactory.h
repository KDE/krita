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

#ifndef KOINLINEOBJECTFACTORY_H
#define KOINLINEOBJECTFACTORY_H

#include <QString>

#include <KoID.h>

#include <koffice_export.h>

class KoInlineObject;

/**
 * A factory for inline text objects. There should be one for each plugin type to
 * allow the creation of the inlineObject from that plugin.
 * The factory additionally has information to allow showing a menu entry for user
 * access to the object-type.
 * @see KoInlineObjectRegistry
 */
class KOTEXT_EXPORT KoInlineObjectFactory : public QObject {
    Q_OBJECT
public:
    /// The type of inlineObject this factory creates.
    enum ObjectType {
        TextVariable,   ///< The factory creates KoVariable inherting objects.
        // etc
        Other = 0x100,  ///< The factory creates objects that should not be shown in any menu
        PopupItem       ///< The factory creates objects that should be show in the PopupMenu
    };

    /**
     * Create the new factory
     * @param parent the parent QObject for memory management usage.
     * @param id a string that will be used internally for referencing the variable-type.
     * @param name the user visible name of the tool this factory creates.
     */
    KoInlineObjectFactory(QObject *parent, const QString &id, const QString &name);
    virtual ~KoInlineObjectFactory() {}

    /**
     * Create a new instance of an inline object.
     */
    virtual KoInlineObject *createInlineObject() = 0;

    /**
     * return the user visible (and translated) name to be seen by the user.
     * @return the user visible (and translated) name to be seen by the user.
     */
    const QString &name() const;

    /**
     * return the id for the variable this factory creates.
     * @return the id for the variable this factory creates.
     */
    const QString &objectId() const;

    /**
     * Create a KoID for the variable this factory creates.
     */
    const KoID id() const;

    /**
     * return the basename of the icon for this inlineObject when its shown in menus
     * @return the basename of the icon for this inlineObject when its shown in menus
     */
    const QString & icon() const;

    /**
     * Returns the type of object this factory creates.
     * The main purpose is to group plugins per type in, for example, a menu.
     * The default returns Other which means it will not be shown in any menu.
     */
    virtual ObjectType type() const { return Other; }

protected:
    /**
     * Set an icon to be used in menus
     * @param iconName the basename (without extension) of the icon
     * @see KIconLoader
     */
    void setIcon(const QString & iconName);

private:
    const QString m_id, m_name;
    QString m_iconName;
};

#endif

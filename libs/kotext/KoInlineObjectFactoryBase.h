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

#include "kotext_export.h"

class KoInlineObject;
class InlineObjectFactoryPrivate;
class KoProperties;

/// A template used in the KoInlineObjectFactoryBase
struct KOTEXT_EXPORT KoInlineObjectTemplate {
    QString id;         ///< The id of the inlineObject
    QString name;       ///< The name to be shown for this template
    /**
     * The properties which, when passed to the KoInlineObjectFactoryBase::createInlineObject() method
     * result in the object this template represents.
     */
    KoProperties *properties;
};

/**
 * A factory for inline text objects. There should be one for each plugin type to
 * allow the creation of the inlineObject from that plugin.
 * The factory additionally has information to allow showing a menu entry for user
 * access to the object-type.
 * @see KoInlineObjectRegistry
 */
class KOTEXT_EXPORT KoInlineObjectFactoryBase : public QObject
{
    Q_OBJECT
public:
    /// The type of inlineObject this factory creates.
    enum ObjectType {
        TextVariable,   ///< The factory creates KoVariable inherting objects.
        Other = 0x100  ///< The factory creates objects that should not be shown in any menu
    };

    /**
     * Create the new factory
     * @param parent the parent QObject for memory management usage.
     * @param id a string that will be used internally for referencing the variable-type.
     */
    KoInlineObjectFactoryBase(QObject *parent, const QString &id, ObjectType type);
    virtual ~KoInlineObjectFactoryBase();

    /**
     * Create a new instance of an inline object.
     */
    virtual KoInlineObject *createInlineObject(const KoProperties *properties = 0) const = 0;

    /**
     * return the id for the variable this factory creates.
     * @return the id for the variable this factory creates.
     */
    QString id() const;

    /**
     * Returns the type of object this factory creates.
     * The main purpose is to group plugins per type in, for example, a menu.
     */
    ObjectType type() const;

    /**
     * Return all the templates this factory knows about.
     * Each template shows a different way to create an object this factory is specialized in.
     */
    QList<KoInlineObjectTemplate> templates() const;

    QStringList odfElementNames() const;

    QString odfNameSpace() const;

    void setOdfElementNames(const QString &nameSpace, const QStringList &names);

protected:
    /**
     * Add a template with the properties of a speficic type of object this factory can generate
     * using the createInlineObject() method.
     * @param params The new template this factory knows to produce
     */
    void addTemplate(const KoInlineObjectTemplate &params);

private:
    InlineObjectFactoryPrivate * const d;
};

#endif

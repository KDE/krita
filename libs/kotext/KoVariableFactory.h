/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOVARIABLEFACTORY_H
#define KOVARIABLEFACTORY_H

#include <QString>

#include <KoID.h>

#include "kotext_export.h"

class KoVariable;
class KoProperties;

/// A template used in the KoVariableFactory
struct KOTEXT_EXPORT KoVariableTemplate {
    QString id;         ///< The id of the inlineObject
    QString name;       ///< The name to be shown for this template
    //QString toolTip;    ///< The tooltip text for the template
    //QString icon;       ///< Icon name
    /**
     * The properties which, when passed to the KoVariableFactory::createInlineObject() method
     * result in the object this template represents.
     */
    KoProperties *properties;
};

/**
 * A factory for inline text objects. There should be one for each plugin type to
 * allow the creation of the inlineObject from that plugin.
 * The factory additionally has information to allow showing a menu entry for user
 * access to the object-type.
 * @see KoVariableRegistry
 */
class KOTEXT_EXPORT KoVariableFactory
{
public:
    /**
     * Create the new factory
     * @param parent the parent QObject for memory management usage.
     * @param id a string that will be used internally for referencing the variable-type.
     */
    KoVariableFactory(const QString &id);
    virtual ~KoVariableFactory();

    /**
     * Create a new instance of an inline object.
     */
    virtual KoVariable * createVariable(const KoProperties *properties) const = 0;

    /**
     * Create an empty variable that can be loaded.
     */
    virtual KoVariable * createVariable() const = 0;

    /**
     * return the id for the variable this factory creates.
     * @return the id for the variable this factory creates.
     */
    const QString & id() const;

    /**
     * Return all the templates this factory knows about.
     * Each template shows a different way to create an object this factory is specialized in.
     */
    const QList<KoVariableTemplate> templates() const;

    QStringList odfElementNames() const;

    const QString & odfNameSpace() const;

protected:
    /**
     * Add a template with the properties of a speficic type of object this factory can generate
     * using the createInlineObject() method.
     * @param params The new template this factory knows to produce
     */
    void addTemplate(const KoVariableTemplate &params);

    void setOdfElementNames(const QString & nameSpace, const QStringList & names);

private:
    class Private;
    Private * const d;
};

#endif /* KOVARIABLEFACTORY_H */

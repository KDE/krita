/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTEDITINGFACTORY_H
#define KOTEXTEDITINGFACTORY_H

#include <QString>
#include <QObject>

#include "kotext_export.h"

class KoTextEditingPlugin;

/**
 * A factory for text editing plugins. There should be one for each plugin type to
 * allow the creation of the text-editing-class from that plugin.
 * @see KoTextEditingRegistry
 */
class KOTEXT_EXPORT KoTextEditingFactory
{
public:
    /**
     * Create the new factory
     * @param parent the parent QObject for memory management usage.
     * @param id a string that will be used internally for referencing the variable-type.
     */
    explicit KoTextEditingFactory(const QString &id);
    virtual ~KoTextEditingFactory();

    /**
     * Create a new instance of an inline object.
     */
    virtual KoTextEditingPlugin *create() const = 0;

    /**
     * return the id for the plugin this factory creates.
     * @return the id for the plugin this factory creates.
     */
    QString id() const;

    /**
     * return if the plugin this factory creates has to be shown in the context menu.
     * @see KoTextEditingPlugin::checkSection()
     */
    bool showInMenu() const;

    /// If showInMenu() returns true; the returned text is used in the context menu.
    QString title() const;

protected:
    /**
     * Set if the plugin this factory creates has to be shown in the context menu.
     * @see KoTextEditingPlugin::checkSection()
     */
    void setShowInMenu(bool show);
    /// set the title used in the context menu
    void setTitle(const QString &title);

private:
    class Private;
    Private * const d;
};

#endif

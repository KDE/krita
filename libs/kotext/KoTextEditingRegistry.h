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

#ifndef KOTEXTEDITINGREGISTRY_H
#define KOTEXTEDITINGREGISTRY_H

#include <KoGenericRegistry.h>
#include <KoTextEditingFactory.h>
#include <QObject>

/**
 * This singleton class keeps a register of all available text editing plugins.
 * The text editing plugins are all about handling user input while (s)he
 * is editing the text. A plugin can do near everything with the typed text,
 * including altering it and adding markup. The plugin gives events when a
 * word and when a paragraph has been finished. Which is ideal for autocorrection
 * and autoreplacement of text.
 * @see KoTextEditingFactory
 * @see KoTextEditingPlugin
 */
class KOTEXT_EXPORT KoTextEditingRegistry : public KoGenericRegistry<KoTextEditingFactory*>
{
public:
    /**
     * Return an instance of the KoTextEditingRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoTextEditingRegistry *instance();

private:
    KoTextEditingRegistry() {}
    virtual ~KoTextEditingRegistry();
    void init();
};

#endif

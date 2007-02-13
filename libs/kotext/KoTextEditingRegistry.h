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

#ifndef KOINLINEOBJECTREGISTRY_H
#define KOINLINEOBJECTREGISTRY_H

#include <KoGenericRegistry.h>
#include <kotext_export.h>
#include <QObject>

class KoTextEditingFactory;

/**
 * This singleton class keeps a register of all available flake shapes,
 * or rather, of the factories that applications can use to create flake
 * shape objects.
 * @see KoInlineObjectFactory
 * @see KoInlineTextObjectManager
 * @see KoInlineObject
 * @see KoVariable
 */
class KOTEXT_EXPORT KoTextEditingRegistry : public QObject,  public KoGenericRegistry<KoTextEditingFactory*>
{
    Q_OBJECT
public:
    /**
     * Return an instance of the KoTextEditingRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoTextEditingRegistry * instance();

private:
    KoTextEditingRegistry() {}
    void init();

private:
    static KoTextEditingRegistry *s_instance;
};

#endif

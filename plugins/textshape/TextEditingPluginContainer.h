/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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
#ifndef TEXTEDITINGPLUGINCONTAINER_H
#define TEXTEDITINGPLUGINCONTAINER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QVariant>

class KoTextEditingPlugin;
class TextTool;

/// This class holds on to the text editing plugins.
/// the goal of this class is to have one plugin instance per koffice-document
/// instead of one per tool.
class TextEditingPluginContainer : public QObject
{
    Q_OBJECT
public:
    enum ResourceManagerId {
        ResourceId = 345681743
    };

    TextEditingPluginContainer(QObject *parent = 0);
    ~TextEditingPluginContainer();

    KoTextEditingPlugin *spellcheck() const;

    KoTextEditingPlugin *plugin(const QString &pluginId) const {
        return m_textEditingPlugins.value(pluginId);
    }

    QList<KoTextEditingPlugin*> values() const {
        return m_textEditingPlugins.values();
    }

private:
    QHash<QString, KoTextEditingPlugin*> m_textEditingPlugins;
};

Q_DECLARE_METATYPE(TextEditingPluginContainer*)

#endif

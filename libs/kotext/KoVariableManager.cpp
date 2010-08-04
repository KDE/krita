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
#include "KoVariableManager.h"
#include "KoInlineTextObjectManager.h"
#include "KoNamedVariable.h"

class KoVariableManagerPrivate
{
public:
    KoVariableManagerPrivate()
            : lastId(KoInlineObject::VariableManagerStart) { }
    KoInlineTextObjectManager *inlineObjectManager;
    QHash<QString, int> variableMapping;
    int lastId;
};

KoVariableManager::KoVariableManager(KoInlineTextObjectManager *inlineObjectManager)
        : d(new KoVariableManagerPrivate)
{
    d->inlineObjectManager = inlineObjectManager;
}

KoVariableManager::~KoVariableManager()
{
    delete d;
}

void KoVariableManager::setValue(const QString &name, const QString &value)
{
    int key;
    // we store the mapping from name to key
    if (d->variableMapping.contains(name))
        key = d->variableMapping.value(name);
    else {
        key = d->lastId++;
        d->variableMapping.insert(name, key);
    }
    // the variable manager stores the actual value of the variable.
    d->inlineObjectManager->setProperty(static_cast<KoInlineObject::Property>(key), value);
    emit valueChanged();
}

QString KoVariableManager::value(const QString &name) const
{
    int key = d->variableMapping.value(name);
    if (key == 0)
        return QString();
    return d->inlineObjectManager->stringProperty(static_cast<KoInlineObject::Property>(key));
}

void KoVariableManager::remove(const QString &name)
{
    int key = d->variableMapping.value(name);
    if (key == 0)
        return;
    d->variableMapping.remove(name);
    d->inlineObjectManager->removeProperty(static_cast<KoInlineObject::Property>(key));
}

int KoVariableManager::usageCount(const QString &name) const
{
    Q_UNUSED(name);
    // TODO
    return 0;
}

KoVariable *KoVariableManager::createVariable(const QString &name) const
{
    int key = d->variableMapping.value(name);
    if (key == 0)
        return 0;
    return new KoNamedVariable(static_cast<KoInlineObject::Property>(key), name);
}

QList<QString> KoVariableManager::variables() const
{
    return d->variableMapping.keys();
}

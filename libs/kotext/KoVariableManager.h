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
#ifndef KOVARIABLEMANAGER_H
#define KOVARIABLEMANAGER_H

#include <QString>

class KoVariable;
class KoInlineTextObjectManager;
class KoVariableManagerPrivate;

/**
 * A document can maintain a list of name-value pairs, which we call variables.
 * These initially exist solely in the variableManager as such name/value pairs.
 * When the user chooses to use one of these pairs in the document he can create a
 * new KoNamedVariable and insert that into the document.  Changing the value here
 * will lead to directly change the value of all variables inserted into the document.
 */
class KoVariableManager {
public:
    explicit KoVariableManager(KoInlineTextObjectManager *inlineObjectManager);

    void setValue(const QString &name, const QString &value);

    void remove(const QString &name);

    QString value(const QString &name) const;

    int usageCount(const QString &name) const;

    KoVariable *createVariable(const QString &name) const;

    QList<QString> variables() const;

private:
    KoVariableManagerPrivate *d;
};

#endif

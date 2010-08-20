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
#ifndef KONAMEDVARIABLE_H
#define KONAMEDVARIABLE_H

#include "KoVariable.h"
#include "kotext_export.h"

/**
 * This inlineObject shows the curent value of a variable as registered in the KoVariableManager.
 * The proper way to create a new class is to use KoVariableManager::createVariable()
 */
class KoNamedVariable : public KoVariable
{
public:
    /// return the name of this named variable
    QString name() const {
        return m_name;
    }

    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    virtual void saveOdf(KoShapeSavingContext &context);

protected:
    friend class KoVariableManager;
    /**
     * Constructor
     * @param key the property that represents the named variable. As defined internally in the KoVariableManager
     * @param name the name of the variable.
     */
    KoNamedVariable(Property key, const QString &name);

private:
    /// reimplemented method
    void propertyChanged(Property property, const QVariant &value);
    /// reimplemented method
    void setup();

    const QString m_name;
    const Property m_key;
};

#endif

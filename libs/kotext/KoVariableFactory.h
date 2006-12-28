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

#ifndef KO_VARIABLE_FACTORY
#define KO_VARIABLE_FACTORY

#include <QString>
//   #include <QWidget>
//   #include <QList>

#include <KoID.h>

#include <koffice_export.h>

class KoVariable;
//   class KoShape;
//   class KoProperties;
//   class KoShapeConfigFactory;
//   class KoShapeConfigWidgetBase;

class KOTEXT_EXPORT KoVariableFactory : public QObject {
    Q_OBJECT
public:

    /**
     * Create the new factory
     * @param parent the parent QObject for memory management usage.
     * @param id a string that will be used internally for referencing the variable-type.
     * @param name the user visible name of the tool this factory creates.
     */
    KoVariableFactory(QObject *parent, const QString &id, const QString &name);
    virtual ~KoVariableFactory() {}

    virtual KoVariable *createVariable() = 0;

    /**
     * return the user visible (and translated) name to be seen by the user.
     * @return the user visible (and translated) name to be seen by the user.
     */
    const QString &name() const;

    /**
     * return the id for the variable this factory creates.
     * @return the id for the variable this factory creates.
     */
    const QString &variableId() const;

    /**
     * Create a KoID for the variable this factory creates.
     */
    const KoID id() const;

private:
    const QString m_id, m_name;
};

#endif

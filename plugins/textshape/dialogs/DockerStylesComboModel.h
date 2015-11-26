/* This file is part of the KDE project
 * Copyright (C) 2012 Pierre Stirnweiss <pstirnweiss@googlemail.org>
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

#ifndef DOCKERSTYLESCOMBOMODEL_H
#define DOCKERSTYLESCOMBOMODEL_H

#include "StylesFilteredModelBase.h"

#include <QVector>

class KoCharacterStyle;
class KoStyleManager;

class DockerStylesComboModel : public StylesFilteredModelBase
{
    Q_OBJECT
public:

    enum CategoriesInternalIds {
        UsedStyleId = -32000,
        UnusedStyleId = -32001
    };

    explicit DockerStylesComboModel(QObject *parent = 0);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void setStyleManager(KoStyleManager *sm);

    void setInitialUsedStyles(QVector<int> usedStyles);

Q_SIGNALS:

public Q_SLOTS:
    void styleApplied(const KoCharacterStyle *style);

protected:
    virtual void createMapping();

private:
    KoCharacterStyle *findStyle(int styleId) const;
    KoStyleManager *m_styleManager;
    QVector<int> m_usedStylesId;
    QVector<int> m_usedStyles;
    QVector<int> m_unusedStyles;
};

#endif // DOCKERSTYLESCOMBOMODEL_H

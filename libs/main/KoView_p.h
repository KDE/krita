/* This file is part of the Calligra project, made within the KDE community.
 * Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#ifndef KOVIEW_P_H
#define KOVIEW_P_H

// Calligra
#include "KoUnit.h"
#include "KoDocument.h"
// Qt
#include <QActionGroup>
#include <QAction>


// Action group which keeps the actions in sync with the document's unit property
class UnitActionGroup : public QActionGroup
{
    Q_OBJECT
public:
    explicit UnitActionGroup(KoDocument *document, bool addPixelUnit, QObject* parent = 0)
            : QActionGroup(parent)
            , m_document(document)
            , m_listOptions(addPixelUnit ? KoUnit::ListAll : KoUnit::HidePixel)
    {
        setExclusive(true);
        connect(this, SIGNAL(triggered(QAction*)), SLOT(onTriggered(QAction*)));
        connect(document, SIGNAL(unitChanged(KoUnit)), SLOT(onUnitChanged(KoUnit)));

        const QStringList unitNames = KoUnit::listOfUnitNameForUi(m_listOptions);
        const int currentUnitIndex = m_document->unit().indexInListForUi(m_listOptions);

        for (int i = 0; i < unitNames.count(); ++i) {
            QAction* action = new QAction(unitNames.at(i), this);
            action->setData(i);
            action->setCheckable(true);

            if (currentUnitIndex == i) {
                action->setChecked(true);
            }
        }
    }

private Q_SLOTS:
    void onTriggered(QAction *action)
    {
        m_document->setUnit(KoUnit::fromListForUi(action->data().toInt(), m_listOptions));
    }

    void onUnitChanged(const KoUnit &unit)
    {
        const int indexInList = unit.indexInListForUi(m_listOptions);

        foreach (QAction *action, actions()) {
            if (action->data().toInt() == indexInList) {
                action->setChecked(true);
                break;
            }
            // in case the new unit is not in the list of actions
            // this ensures that the action currently checked is unchecked
            // once the end of actions has been reached
            if (action->isChecked()) {
                action->setChecked(false);
            }
        }
    }

private:
    KoDocument *m_document;
    KoUnit::ListOptions m_listOptions;
};

#endif

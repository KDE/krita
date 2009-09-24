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

#ifndef KOVIEW_P_H
#define KOVIEW_P_H

#include "KoUnit.h"
#include "KoDocument.h"

#include <QAction>

class UnitChangeAction : public QAction
{
    Q_OBJECT
public:
    UnitChangeAction(KoUnit::Unit unit, QObject *parent, KoDocument *document)
            : QAction(KoUnit::unitDescription(KoUnit(unit)), parent),
            m_document(document),
            m_unit(unit) {
        setCheckable(true);
        connect(this, SIGNAL(triggered(bool)), SLOT(activated()));
    }

public slots:
    void activated() {
        m_document->setUnit(m_unit);
    }

private:
    KoDocument *m_document;
    KoUnit m_unit;
};

#endif

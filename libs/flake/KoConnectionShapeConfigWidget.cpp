/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoConnectionShapeConfigWidget.h"
#include "commands/KoConnectionShapeTypeCommand.h"
#include <klocale.h>

KoConnectionShapeConfigWidget::KoConnectionShapeConfigWidget()
{
    widget.setupUi(this);

    widget.connectionType->clear();
    widget.connectionType->addItem(i18n("Standard"));
    widget.connectionType->addItem(i18n("Lines"));
    widget.connectionType->addItem(i18n("Straight"));
    widget.connectionType->addItem(i18n("Curve"));

    connect(widget.connectionType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(propertyChanged()));
}

void KoConnectionShapeConfigWidget::open(KoShape *shape)
{
    m_connection = dynamic_cast<KoConnectionShape*>(shape);
    if (! m_connection)
        return;

    widget.connectionType->blockSignals(true);

    widget.connectionType->setCurrentIndex(m_connection->type());

    widget.connectionType->blockSignals(false);
}

void KoConnectionShapeConfigWidget::save()
{
    if (! m_connection)
        return;

    m_connection->setType(static_cast<KoConnectionShape::Type>(widget.connectionType->currentIndex()));
}

QUndoCommand * KoConnectionShapeConfigWidget::createCommand()
{
    if (! m_connection)
        return 0;
    else {
        KoConnectionShape::Type type = static_cast<KoConnectionShape::Type>(widget.connectionType->currentIndex());
        return new KoConnectionShapeTypeCommand(m_connection, type);
    }
}

#include <KoConnectionShapeConfigWidget.moc>

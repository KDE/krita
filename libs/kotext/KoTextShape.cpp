/* This file is part of the KDE project
 * Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoTextShape.h"

#include <QTextLayout>
#include <QFont>
#include <QAbstractTextDocumentLayout>
#include <kdebug.h>


// ############ KoTextShape ################

KoTextShape::KoTextShape()
{
    m_textShapeData = new KoTextShapeData();
    setUserData(m_textShapeData);
}

KoTextShape::~KoTextShape() {
}

void KoTextShape::paint(QPainter &painter, KoViewConverter &converter) {
    painter.fillRect(converter.documentToView(QRectF(QPointF(0.0,0.0), size())), background());
    applyConversion(painter, converter);
    QAbstractTextDocumentLayout::PaintContext pc;
    pc.cursorPosition = -1;

    QTextDocument *doc = m_textShapeData->document();
    painter.translate(0, -m_textShapeData->documentOffset());
    doc->documentLayout()->draw( &painter, pc);
}

QPointF KoTextShape::convertScreenPos(const QPointF &point) {
    return m_invMatrix.map(point);
}

void KoTextShape::shapeChanged(ChangeType type) {
    if(type == KoShape::SizeChanged)
        m_textShapeData->document()->setPageSize(size());
}

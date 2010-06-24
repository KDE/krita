/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2010 Thomas Zander <zander@kde.org>
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

#include "KoTextShapeDataBase.h"
#include "KoTextShapeDataBase_p.h"

KoTextShapeDataBasePrivate::KoTextShapeDataBasePrivate()
        : document(0),
        textAlignment(Qt::AlignLeft | Qt::AlignTop)
{
}

KoTextShapeDataBasePrivate::~KoTextShapeDataBasePrivate()
{
}

KoTextShapeDataBase::KoTextShapeDataBase(KoTextShapeDataBasePrivate &dd)
    : d_ptr(&dd)
{
}

KoTextShapeDataBase::~KoTextShapeDataBase()
{
    delete d_ptr;
}

QTextDocument *KoTextShapeDataBase::document() const
{
    Q_D(const KoTextShapeDataBase);
    return d->document;
}

void KoTextShapeDataBase::setShapeMargins(const KoInsets &margins)
{
    Q_D(KoTextShapeDataBase);
    d->margins = margins;
}

KoInsets KoTextShapeDataBase::shapeMargins() const
{
    Q_D(const KoTextShapeDataBase);
    return d->margins;
}

void KoTextShapeDataBase::setVerticalAlignment(Qt::Alignment alignment)
{
    Q_D(KoTextShapeDataBase);
    d->textAlignment = (d->textAlignment & Qt::AlignHorizontal_Mask)
        | (alignment & Qt::AlignVertical_Mask);
}

Qt::Alignment KoTextShapeDataBase::verticalAlignment() const
{
    Q_D(const KoTextShapeDataBase);
    return d->textAlignment & Qt::AlignVertical_Mask;
}

#include <KoTextShapeDataBase.moc>

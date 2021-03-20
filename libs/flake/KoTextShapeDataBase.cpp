/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2009-2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoTextShapeDataBase.h"
#include "KoTextShapeDataBase_p.h"

#include <QTextDocument>

KoTextShapeDataBasePrivate::KoTextShapeDataBasePrivate()
        : textAlignment(Qt::AlignLeft | Qt::AlignTop)
        , resizeMethod(KoTextShapeDataBase::NoResize)
{
}

KoTextShapeDataBasePrivate::KoTextShapeDataBasePrivate(const KoTextShapeDataBasePrivate &rhs)
    : document(rhs.document->clone()),
      margins(rhs.margins),
      textAlignment(rhs.textAlignment),
      resizeMethod(rhs.resizeMethod)
{
}

KoTextShapeDataBasePrivate::~KoTextShapeDataBasePrivate()
{
}

KoTextShapeDataBase::KoTextShapeDataBase(KoTextShapeDataBasePrivate *dd)
    : d_ptr(dd)
{
}

KoTextShapeDataBase::~KoTextShapeDataBase()
{
    delete d_ptr;
}

QTextDocument *KoTextShapeDataBase::document() const
{
    Q_D(const KoTextShapeDataBase);
    return d->document.data();
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

void KoTextShapeDataBase::setResizeMethod(KoTextShapeDataBase::ResizeMethod method)
{
    Q_D(KoTextShapeDataBase);
    if (d->resizeMethod == method)
        return;
    d->resizeMethod = method;
}

KoTextShapeDataBase::ResizeMethod KoTextShapeDataBase::resizeMethod() const
{
    Q_D(const KoTextShapeDataBase);
    return d->resizeMethod;
}

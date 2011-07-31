/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008,2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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
#include "KoTableBorderStyle.h"
/*
KoTableBorderStylePrivate::KoTableBorderStylePrivate()
{
    edges[KoTableBorderStyle::Top].spacing = 0;
    borderstyle[KoTableBorderStyle::Top] = KoBorder::BorderNone;

    edges[KoTableBorderStyle::Left].spacing = 0;
    borderstyle[KoTableBorderStyle::Left] = KoBorder::BorderNone;

    edges[KoTableBorderStyle::Bottom].spacing = 0;
    borderstyle[KoTableBorderStyle::Bottom] = KoBorder::BorderNone;

    edges[KoTableBorderStyle::Right].spacing = 0;
    borderstyle[KoTableBorderStyle::Right] = KoBorder::BorderNone;

    edges[KoTableBorderStyle::TopLeftToBottomRight].spacing = 0;
    borderstyle[KoTableBorderStyle::TopLeftToBottomRight] = KoBorder::BorderNone;

    edges[KoTableBorderStyle::BottomLeftToTopRight].spacing = 0;
    borderstyle[KoTableBorderStyle::BottomLeftToTopRight] = KoBorder::BorderNone;
}

KoTableBorderStylePrivate::~KoTableBorderStylePrivate()
{
}

KoTableBorderStyle::KoTableBorderStyle(QObject *parent)
    : QObject(parent)
    , d_ptr(new KoTableBorderStylePrivate())
{
}

KoTableBorderStyle::KoTableBorderStyle(const QTextTableCellFormat &format, QObject *parent)
    : QObject(parent)
    , d_ptr(new KoTableBorderStylePrivate())
{
    init(format);
}

KoTableBorderStyle::KoTableBorderStyle(KoTableBorderStylePrivate &dd, const QTextTableCellFormat &format, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    init(format);
}

KoTableBorderStyle::KoTableBorderStyle(KoTableBorderStylePrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
}

void KoTableBorderStyle::init(const QTextTableCellFormat &format)
{
    Q_D(KoTableBorderStyle);

    d->edges[Top].outerPen = format.penProperty(TopBorderOuterPen);
    d->edges[Top].spacing = format.doubleProperty(TopBorderSpacing);
    d->edges[Top].innerPen = format.penProperty(TopBorderInnerPen);
    d->borderstyle[Top] = KoBorder::BorderStyle(format.intProperty(TopBorderStyle));

    d->edges[Left].outerPen = format.penProperty(LeftBorderOuterPen);
    d->edges[Left].spacing = format.doubleProperty(LeftBorderSpacing);
    d->edges[Left].innerPen = format.penProperty(LeftBorderInnerPen);
    d->borderstyle[Left] = KoBorder::BorderStyle(format.intProperty(LeftBorderStyle));

    d->edges[Bottom].outerPen =format.penProperty(BottomBorderOuterPen);
    d->edges[Bottom].spacing = format.doubleProperty(BottomBorderSpacing);
    d->edges[Bottom].innerPen = format.penProperty(BottomBorderInnerPen);
    d->borderstyle[Bottom] = KoBorder::BorderStyle(format.intProperty(BottomBorderStyle));

    d->edges[Right].outerPen = format.penProperty(RightBorderOuterPen);
    d->edges[Right].spacing = format.doubleProperty(RightBorderSpacing);
    d->edges[Right].innerPen = format.penProperty(RightBorderInnerPen);
    d->borderstyle[Right] = KoBorder::BorderStyle(format.intProperty(RightBorderStyle));

    d->edges[TopLeftToBottomRight].outerPen = format.penProperty(TopLeftToBottomRightBorderOuterPen);
    d->edges[TopLeftToBottomRight].spacing = format.doubleProperty(TopLeftToBottomRightBorderSpacing);
    d->edges[TopLeftToBottomRight].innerPen = format.penProperty(TopLeftToBottomRightBorderInnerPen);
    d->borderstyle[TopLeftToBottomRight] = KoBorder::BorderStyle(format.intProperty(TopLeftToBottomRightBorderStyle));

    d->edges[BottomLeftToTopRight].outerPen = format.penProperty(BottomLeftToTopRightBorderOuterPen);
    d->edges[BottomLeftToTopRight].spacing = format.doubleProperty(BottomLeftToTopRightBorderSpacing);
    d->edges[BottomLeftToTopRight].innerPen = format.penProperty(BottomLeftToTopRightBorderInnerPen);
    d->borderstyle[BottomLeftToTopRight] = KoBorder::BorderStyle(format.intProperty(BottomLeftToTopRightBorderStyle));
}

KoTableBorderStyle::~KoTableBorderStyle()
{
    delete d_ptr;
}
*/
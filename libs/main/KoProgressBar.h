/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2007
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
#ifndef KOPROGRESSBAR_H
#define KOPROGRESSBAR_H

#include <QProgressBar>
#include <KoProgressProxy.h>
#include "komain_export.h"

/**
 * KoProgressBar is a thin wrapper around QProgressBar that also implements
 * the abstract base class KoProgressProxy. Use this class, not QProgressBar
 * to pass to KoProgressUpdater.
 */
class KOMAIN_EXPORT KoProgressBar : public QProgressBar, public KoProgressProxy
{
public:

    KoProgressBar( QWidget * parent = 0 );

    ~KoProgressBar();

    int maximum() const;
    void setValue( int value );
    void setRange( int minimum, int maximum );
    void setFormat( const QString & format );
};

#endif

/* This file is part of the KDE project
 * Copyright (c) 1999 Carsten Pfeiffer (pfeiffer@kde.org)
 * Copyright (c) 2002 Igor Jansen (rm@kde.org)
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISICONTOOLTIP_H
#define KISICONTOOLTIP_H

#include "KoItemToolTip.h"

#include "kritaresourcewidgets_export.h"

class KRITARESOURCEWIDGETS_EXPORT KisIconToolTip: public KoItemToolTip
{

public:
    KisIconToolTip();
    ~KisIconToolTip() override;

protected:
    QTextDocument *createDocument( const QModelIndex &index ) override;
};

#endif // KOICONTOOLTIP_H

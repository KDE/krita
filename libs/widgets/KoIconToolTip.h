/* This file is part of the KDE project
  Copyright (c) 1999 Carsten Pfeiffer (pfeiffer@kde.org)
  Copyright (c) 2002 Igor Jansen (rm@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOICONTOOLTIP_H
#define KOICONTOOLTIP_H

#include "KoItemToolTip.h"

#include "kritawidgets_export.h"

class KRITAWIDGETS_EXPORT KoIconToolTip: public KoItemToolTip
{

    public:
        KoIconToolTip();
        ~KoIconToolTip() override;

    protected:
        QTextDocument *createDocument( const QModelIndex &index ) override;

    private:
        typedef KoItemToolTip super;
};

#endif // KOICONTOOLTIP_H

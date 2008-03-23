/*
 *  Copyright (c) 2007 Emanuele Tamponi (emanuele@valinor.it)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_COMPLEXOP_H_
#define KIS_COMPLEXOP_H_

#include <kis_paintop.h>

class KisPainter;
class KisPaintInformation;
class KisPaintOpSettings;
class QString;

class KisComplexOp : public KisPaintOp {

    typedef KisPaintOp super;

    public:

        KisComplexOp(KisPainter *painter);
        ~KisComplexOp();

        void paintAt(const KisPaintInformation &info);
        bool painterly() const {return true;}

};


class KisComplexOpFactory : public KisPaintOpFactory  {

    public:
        KisComplexOpFactory() {}
        ~KisComplexOpFactory() {}

        KisPaintOp *createOp(const KisPaintOpSettings *settings, KisPainter *painter, KisImageSP image);
        QString id() const;
        QString name() const;

};

#endif // KIS_COMPLEXOP_H_

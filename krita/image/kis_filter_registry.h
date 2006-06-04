/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_FILTER_REGISTRY_H_
#define KIS_FILTER_REGISTRY_H_

#include <QObject>

#include "kis_types.h"
#include "KoGenericRegistry.h"

#include <krita_export.h>

class QString;
class QStringList;

class KRITAIMAGE_EXPORT KisFilterRegistry : public QObject, public KisGenericRegistry<KisFilterSP>
{

    Q_OBJECT

public:
    virtual ~KisFilterRegistry();

    static KisFilterRegistry* instance();

private:
    KisFilterRegistry();
    KisFilterRegistry(const KisFilterRegistry&);
    KisFilterRegistry operator=(const KisFilterRegistry&);

private:
     static KisFilterRegistry *m_singleton;
};

#endif // KIS_FILTERSPACE_REGISTRY_H_

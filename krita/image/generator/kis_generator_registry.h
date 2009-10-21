/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_GENERATOR_REGISTRY_H_
#define KIS_GENERATOR_REGISTRY_H_

#include <QObject>

#include "kis_types.h"
#include "KoGenericRegistry.h"

#include <krita_export.h>

class QString;

/**
 * XXX_DOCS
 */
class KRITAIMAGE_EXPORT KisGeneratorRegistry : public QObject, public KoGenericRegistry<KisGeneratorSP>
{

    Q_OBJECT

public:
    virtual ~KisGeneratorRegistry();

    static KisGeneratorRegistry* instance();
    void add(KisGeneratorSP item);
    void add(const QString &id, KisGeneratorSP item);

signals:

    void generatorAdded(QString id);

private:

    KisGeneratorRegistry();
    KisGeneratorRegistry(const KisGeneratorRegistry&);
    KisGeneratorRegistry operator=(const KisGeneratorRegistry&);

};

#endif // KIS_GENERATOR_REGISTRY_H_

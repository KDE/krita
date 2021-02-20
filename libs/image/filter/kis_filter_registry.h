/*
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTER_REGISTRY_H_
#define KIS_FILTER_REGISTRY_H_

#include <QObject>

#include "kis_filter.h"
#include "kis_types.h"
#include "KoGenericRegistry.h"

#include <kritaimage_export.h>

class QString;
class KisFilterConfiguration;

class KRITAIMAGE_EXPORT KisFilterRegistry : public QObject, public KoGenericRegistry<KisFilterSP>
{

    Q_OBJECT

public:

    ~KisFilterRegistry() override;

    static KisFilterRegistry* instance();
    void add(KisFilterSP item);
    void add(const QString &id, KisFilterSP item);

Q_SIGNALS:

    void filterAdded(QString id);

private:

    KisFilterRegistry(QObject *parent);
    KisFilterRegistry(const KisFilterRegistry&);
    KisFilterRegistry operator=(const KisFilterRegistry&);

};

#endif // KIS_FILTERSPACE_REGISTRY_H_

/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GENERATOR_REGISTRY_H_
#define KIS_GENERATOR_REGISTRY_H_

#include <QObject>

#include "kis_generator.h"
#include "kis_types.h"
#include "KoGenericRegistry.h"

#include <kritaimage_export.h>

class QString;
class KisFilterConfiguration;

/**
 * XXX_DOCS
 */
class KRITAIMAGE_EXPORT KisGeneratorRegistry : public QObject, public KoGenericRegistry<KisGeneratorSP>
{

    Q_OBJECT

public:
    ~KisGeneratorRegistry() override;

    static KisGeneratorRegistry* instance();
    void add(KisGeneratorSP item);
    void add(const QString &id, KisGeneratorSP item);

Q_SIGNALS:

    void generatorAdded(QString id);

private:

    KisGeneratorRegistry(QObject *parent);
    KisGeneratorRegistry(const KisGeneratorRegistry&);
    KisGeneratorRegistry operator=(const KisGeneratorRegistry&);
};

#endif // KIS_GENERATOR_REGISTRY_H_

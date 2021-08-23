/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <KoGenericRegistry.h>
#include <kis_meta_data_io_backend.h>

#include "kritametadata_export.h"

class KRITAMETADATA_EXPORT KisMetadataBackendRegistry : public KoGenericRegistry<KisMetaData::IOBackend *>
{
public:
    KisMetadataBackendRegistry();
    ~KisMetadataBackendRegistry() override;
    void init();
    static KisMetadataBackendRegistry *instance();
};

/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_io_backend.h"

using namespace KisMetaData;

#include <QGlobalStatic>
#include "kis_debug.h"

Q_GLOBAL_STATIC(IOBackendRegistry, s_instance)


IOBackendRegistry::IOBackendRegistry()
{
}

IOBackendRegistry::~IOBackendRegistry()
{
    Q_FOREACH (const QString &id, keys()) {
        delete get(id);
    }    
}


IOBackendRegistry* IOBackendRegistry::instance()
{
    // XXX: load backend plugins
    return s_instance;
}

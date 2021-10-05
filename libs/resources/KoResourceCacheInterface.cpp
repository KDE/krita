/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoResourceCacheInterface.h"


struct KoResourceCacheInterfaceSPStaticRegistrar {
    KoResourceCacheInterfaceSPStaticRegistrar() {
        qRegisterMetaType<KoResourceCacheInterfaceSP>("KoResourceCacheInterfaceSP");
        QMetaType::registerEqualsComparator<KoResourceCacheInterfaceSP>();
    }
};
static KoResourceCacheInterfaceSPStaticRegistrar __registrar1;

KoResourceCacheInterface::~KoResourceCacheInterface()
{

}

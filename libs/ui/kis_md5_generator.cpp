/*
 * SPDX-FileCopyrightText: 2015 Stefano Bonicatti <smjert@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include "kis_md5_generator.h"

#include <kis_debug.h>
#include <KoStore.h>

KisMD5Generator::KisMD5Generator()
{

}

KisMD5Generator::~KisMD5Generator()
{

}

QByteArray KisMD5Generator::generateHash(const QString &filename)
{
    QByteArray ba;
    if(filename.startsWith("bundle://")) {
        QString bn = filename.mid(9);
        int pos = bn.lastIndexOf(":");
        QString fn = bn.right(bn.size() - pos - 1);
        bn = bn.left(pos);

        QScopedPointer<KoStore> resourceStore(KoStore::createStore(bn, KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));
        if (!resourceStore || resourceStore->bad()) {
            warnKrita << "Could not open store on bundle" << bn;
            return ba;
        }

        if (resourceStore->isOpen()) resourceStore->close();

        if (!resourceStore->open(fn)) {
            warnKrita << "Could not open preset" << fn << "in bundle" << bn;
            return ba;
        }

        ba = resourceStore->device()->readAll();

        resourceStore->close();
        return KoMD5Generator::generateHash(ba);

    }

    return KoMD5Generator::generateHash(filename);
}

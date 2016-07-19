/*
 * Copyright (c) 2015 Stefano Bonicatti <smjert@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

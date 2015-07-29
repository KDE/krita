/*
 *  Copyright (c) 2015 Stefano Bonicatti <smjert@gmail.com>
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
#include "KoHashGeneratorProvider.h"

#include <QMutexLocker>

#include "KoMD5Generator.h"

KoHashGeneratorProvider *KoHashGeneratorProvider::instance_var = 0;

KoHashGeneratorProvider::KoHashGeneratorProvider()
{
    // Initialize default generators
    hashGenerators.insert("MD5", new KoMD5Generator());
}

KoHashGeneratorProvider::~KoHashGeneratorProvider()
{
    qDeleteAll(hashGenerators);
}

KoHashGenerator *KoHashGeneratorProvider::getGenerator(QString algorithm)
{
    QMutexLocker locker(&mutex);
    return hashGenerators.value(algorithm);
}

void KoHashGeneratorProvider::setGenerator(QString algorithm, KoHashGenerator *generator)
{
    if(hashGenerators.contains(algorithm)) {
        hashGenerators[algorithm] = generator;
    }
    else
        hashGenerators.insert(algorithm, generator);
}

KoHashGeneratorProvider *KoHashGeneratorProvider::instance()
{
    if(!instance_var)
        instance_var = new KoHashGeneratorProvider();

    return instance_var;
}

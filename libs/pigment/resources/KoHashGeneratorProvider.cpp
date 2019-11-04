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
#include "KoHashGeneratorProvider.h"

#include <QMutexLocker>
#include <QGlobalStatic>

#include "KoMD5Generator.h"

KoHashGeneratorProvider *KoHashGeneratorProvider::instance_var = 0;
Q_GLOBAL_STATIC(KoHashGeneratorProvider, s_instance)
    
KoHashGeneratorProvider::KoHashGeneratorProvider()
{
    // Initialize default generators
    hashGenerators.insert("MD5", new KoMD5Generator());
}

KoHashGeneratorProvider::~KoHashGeneratorProvider()
{
    qDeleteAll(hashGenerators);
}

KoHashGenerator *KoHashGeneratorProvider::getGenerator(const QString &algorithm)
{
    QMutexLocker locker(&mutex);
    return hashGenerators.value(algorithm);
}

void KoHashGeneratorProvider::setGenerator(const QString &algorithm, KoHashGenerator *generator)
{
    if (hashGenerators.contains(algorithm)) {
        delete hashGenerators.take(algorithm);
        hashGenerators[algorithm] = generator;
    }
    else
        hashGenerators.insert(algorithm, generator);
}

KoHashGeneratorProvider *KoHashGeneratorProvider::instance()
{
    return s_instance;
}

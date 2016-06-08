/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoAnnotationManager.h"
#include "KoAnnotation.h"

#include "TextDebug.h"
#include <QHash>

class KoAnnotationManagerPrivate
{
public:
    KoAnnotationManagerPrivate() { }
    QHash<QString, KoAnnotation*> annotationHash;
    QList<QString> annotationNameList;
    int lastId;
};

KoAnnotationManager::KoAnnotationManager()
        : d(new KoAnnotationManagerPrivate)
{
}

KoAnnotationManager::~KoAnnotationManager()
{
    delete d;
}

void KoAnnotationManager::insert(const QString &name, KoAnnotation *annotation)
{
    annotation->setName(name);
    d->annotationHash[name] = annotation;
    d->annotationNameList.append(name);
}

void KoAnnotationManager::remove(const QString &name)
{
    d->annotationHash.remove(name);
    d->annotationNameList.removeAt(d->annotationNameList.indexOf(name));
}

void KoAnnotationManager::rename(const QString &oldName, const QString &newName)
{
    QHash<QString, KoAnnotation*>::iterator i = d->annotationHash.begin();

    while (i != d->annotationHash.end()) {
        if (i.key() == oldName) {
            KoAnnotation *annotation = d->annotationHash.take(i.key());
            annotation->setName(newName);
            d->annotationHash.insert(newName, annotation);
            int listPos = d->annotationNameList.indexOf(oldName);
            d->annotationNameList.replace(listPos, newName);
            return;
        }
        ++i;
    }
}

KoAnnotation *KoAnnotationManager::annotation(const QString &name) const
{
    KoAnnotation *annotation = d->annotationHash.value(name);
    return annotation;
}

QList<QString> KoAnnotationManager::annotationNameList() const
{
    return d->annotationNameList;
}

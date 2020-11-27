/* This file is part of the KDE project
 * Copyright (c) 2010 Boudewijn Rempt (boud@valdyas.org)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoDeferredShapeFactoryBase.h"

KoDeferredShapeFactoryBase::KoDeferredShapeFactoryBase(QObject *parent)
    : QObject(parent)
{

}

KoDeferredShapeFactoryBase::~KoDeferredShapeFactoryBase()
{

}

KoShape *KoDeferredShapeFactoryBase::createShape(const KoProperties *, KoDocumentResourceManager *documentResources) const
{
    return createDefaultShape(documentResources);
}

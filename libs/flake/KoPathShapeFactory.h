/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Rob Buis <buis@kde.org>
   SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOPATHSHAPEFACTORY_H
#define KOPATHSHAPEFACTORY_H

#include "KoShapeFactoryBase.h"

#include "KoXmlReader.h"

class KoShape;

/// Factory for path shapes.
class KRITAFLAKE_EXPORT KoPathShapeFactory : public KoShapeFactoryBase
{
public:
    /// constructor
    KoPathShapeFactory(const QStringList&);
    ~KoPathShapeFactory() override {}
    KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const override;
    bool supports(const KoXmlElement &element, KoShapeLoadingContext &context) const override;
    /// reimplemented
    void newDocumentResourceManager(KoDocumentResourceManager *manager) const override;
};

#endif

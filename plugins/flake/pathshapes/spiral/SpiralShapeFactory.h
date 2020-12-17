/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Rob Buis <buis@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSPIRALSHAPEFACTORY_H
#define KOSPIRALSHAPEFACTORY_H

#include "KoShapeFactoryBase.h"

class KoShape;

/// Factory for spiral shapes
class SpiralShapeFactory : public KoShapeFactoryBase
{
public:
    /// constructor
    SpiralShapeFactory();
    ~SpiralShapeFactory() override {}
    KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const override;
    bool supports(const KoXmlElement &e, KoShapeLoadingContext &context) const override;
    QList<KoShapeConfigWidgetBase *> createShapeOptionPanels() override;
};

#endif /* KOSPIRALSHAPEFACTORY_H */

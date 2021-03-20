/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOELLIPSESHAPEFACTORY_H
#define KOELLIPSESHAPEFACTORY_H

#include "KoShapeFactoryBase.h"
#include <QDebug>

class KoShape;

/// Factory for ellipse shapes
class EllipseShapeFactory : public KoShapeFactoryBase
{
public:
    /// constructor
    EllipseShapeFactory();
    ~EllipseShapeFactory() override {}
    KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const override;
    bool supports(const KoXmlElement &e, KoShapeLoadingContext &context) const override;
    QList<KoShapeConfigWidgetBase *> createShapeOptionPanels() override;
};

#endif /* KOELLIPSESHAPEFACTORY_H */

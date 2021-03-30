/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006-2007 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSTARHAPEFACTORY_H
#define KOSTARHAPEFACTORY_H

#include <KoShapeFactoryBase.h>

class KoShape;

/// Factory for path shapes
class StarShapeFactory : public KoShapeFactoryBase
{
public:
    /// constructor
    StarShapeFactory();
    ~StarShapeFactory() override {}
    KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const override;
    KoShape *createShape(const KoProperties *params, KoDocumentResourceManager *documentResources = 0) const override;
    bool supports(const QDomElement &e, KoShapeLoadingContext &context) const override;
    QList<KoShapeConfigWidgetBase *> createShapeOptionPanels() override;
};

#endif // KOSTARHAPEFACTORY_H

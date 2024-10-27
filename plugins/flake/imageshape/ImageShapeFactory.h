/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2009 Inge Wallin <inge@lysator.liu.se>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef IMAGESHAPE_FACTORY_H
#define IMAGESHAPE_FACTORY_H

// Calligra
#include <KoShapeFactoryBase.h>

class KoShape;

class ImageShapeFactory : public KoShapeFactoryBase
{

public:
    /// constructor
    ImageShapeFactory();
    ~ImageShapeFactory() override {}

    KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const override;
    /**
     * @brief createShape
     * @param params
     * The parameters. Will look at keys...
     * - "image" and retrieve as a QImage to set on the shape.
     * - "viewboxTransform" and retrieve as a QTransform to set on the shape.
     * @param documentResources -- unused.
     * @return ImageShape
     */
    KoShape *createShape(const KoProperties *params, KoDocumentResourceManager *documentResources = 0) const;
    bool supports(const QDomElement &e, KoShapeLoadingContext &context) const override;
    QList<KoShapeConfigWidgetBase *> createShapeOptionPanels() override;
};

#endif

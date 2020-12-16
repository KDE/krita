/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Fela Winkelmolen <fela.kde@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KarbonCalligraphicShapeFactory.h"
#include "KarbonCalligraphicShape.h"

#include <KoIcon.h>
#include <klocalizedstring.h>
#include <KoShapeLoadingContext.h>

KarbonCalligraphicShapeFactory::KarbonCalligraphicShapeFactory()
    : KoShapeFactoryBase(KarbonCalligraphicShapeId, i18n("A calligraphic shape"))
{
    setToolTip(i18n("Calligraphic Shape"));
    setIconName(koIconNameCStr("calligraphy"));
    setLoadingPriority(1);
    setHidden(true);
}

KarbonCalligraphicShapeFactory::~KarbonCalligraphicShapeFactory()
{
}

KoShape *KarbonCalligraphicShapeFactory::createDefaultShape(KoDocumentResourceManager *) const
{
    KarbonCalligraphicShape *path = new KarbonCalligraphicShape();

    // FIXME: add points
    path->setShapeId(KarbonCalligraphicShapeId);

    return path;
}

bool KarbonCalligraphicShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(e);
    Q_UNUSED(context);
    return false;
}

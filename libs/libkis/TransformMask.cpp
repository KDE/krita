/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "TransformMask.h"
#include <kis_transform_mask.h>
#include <kis_image.h>
#include <kis_transform_mask_params_interface.h>
#include <QDomDocument>

#include "Node.h"
#include <kis_transform_mask_params_factory_registry.h>
#include <commands_new/KisSimpleModifyTransformMaskCommand.h>
#include <kis_processing_applicator.h>


#include "kis_transform_mask_params_interface.h"


TransformMask::TransformMask(KisImageSP image, QString name, QObject *parent) :
    Node(image, new KisTransformMask(image, name), parent)
{

}

TransformMask::TransformMask(KisImageSP image, KisTransformMaskSP mask, QObject *parent):
    Node(image, mask, parent)
{

}

TransformMask::~TransformMask()
{

}

QString TransformMask::type() const
{
    return "transformmask";
}

QTransform TransformMask::finalAffineTransform() const
{
    QTransform affineTransformation;
    KisTransformMask *mask = dynamic_cast<KisTransformMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, QTransform());

    affineTransformation = mask->transformParams()->finalAffineTransform();

    return affineTransformation;
}

QString TransformMask::toXML() const
{
    KisTransformMaskSP mask = qobject_cast<KisTransformMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, QString());

    QDomDocument doc("transform_params");
    QDomElement root = doc.createElement("transform_params");
    QDomElement main = doc.createElement("main");
    QDomElement data = doc.createElement("data");

    main.setAttribute("id", mask->transformParams()->id());

    mask->transformParams()->toXML(&data);
    doc.appendChild(root);
    root.appendChild(main);
    root.appendChild(data);

    return doc.toString();
}

bool TransformMask::fromXML (const QString &xml)
{
    QDomDocument doc;
    KisTransformMaskSP mask = qobject_cast<KisTransformMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, false);

    doc.setContent(xml);

    QDomElement rootElement = doc.documentElement();
    QDomElement main = rootElement.firstChildElement("main");
    QDomElement data = rootElement.firstChildElement("data");

    if (!main.isElement() || !main.hasAttribute("id") || !data.isElement()) {
        return false;
    }

    KisTransformMaskParamsInterfaceSP params = KisTransformMaskParamsFactoryRegistry::instance()->createParams(main.attribute("id"), data);

    if (!params) {
        return false;
    }

    KUndo2Command *cmd = new KUndo2Command();

    if (mask->isAnimated()) {
        KisAnimatedTransformParamsInterface* animInterface = dynamic_cast<KisAnimatedTransformParamsInterface*>(mask->transformParams().data());
        KIS_ASSERT(animInterface);
        animInterface->initializeKeyframes(mask, params, cmd);
    } else {
        cmd = new KisSimpleModifyTransformMaskCommand(mask, mask->transformParams(), params);
    }

    KisProcessingApplicator::runSingleCommandStroke(this->node()->image(), cmd);

    return true;
}

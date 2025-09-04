/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_TRANSFORMMASK_H
#define LIBKIS_TRANSFORMMASK_H

#include <QObject>
#include "Node.h"

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief The TransformMask class
 * A transform mask is a mask type node that can be used
 * to store transformations.
 */
class KRITALIBKIS_EXPORT TransformMask : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(TransformMask)

public:
    explicit TransformMask(KisImageSP image, QString name, QObject *parent = 0);
    explicit TransformMask(KisImageSP image, KisTransformMaskSP mask, QObject *parent = 0);
    ~TransformMask() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return transformmask
     *
     * If the Node object isn't wrapping a valid Krita layer or mask object, and
     * empty string is returned.
     */
    virtual QString type() const override;

    QTransform finalAffineTransform() const;

    /**
     * @brief toXML
     * @return a string containing XML formatted transform parameters.
     */
    QString toXML() const;

    /**
     * @brief fromXML set the transform of the transform mask from XML formatted data.
     * The xml must have a valid id
     *
     * dumbparams - placeholder for static transform masks
     * tooltransformparams - static transform mask
     * animatedtransformparams - animated transform mask
     *
@code
<!DOCTYPE transform_params>
<transform_params>
  <main id="tooltransformparams"/>
  <data mode="0">
   <free_transform>
    <transformedCenter type="pointf" x="12.3102137276208" y="11.0727768562035"/>
    <originalCenter type="pointf" x="20" y="20"/>
    <rotationCenterOffset type="pointf" x="0" y="0"/>
    <transformAroundRotationCenter value="0" type="value"/>
    <aX value="0" type="value"/>
    <aY value="0" type="value"/>
    <aZ value="0" type="value"/>
    <cameraPos z="1024" type="vector3d" x="0" y="0"/>
    <scaleX value="1" type="value"/>
    <scaleY value="1" type="value"/>
    <shearX value="0" type="value"/>
    <shearY value="0" type="value"/>
    <keepAspectRatio value="0" type="value"/>
    <flattenedPerspectiveTransform m23="0" m31="0" m32="0" type="transform" m33="1" m12="0" m13="0" m22="1" m11="1" m21="0"/>
    <filterId value="Bicubic" type="value"/>
   </free_transform>
  </data>
</transform_params>
@endcode
     * @param xml a valid formatted XML string with proper main and data elements.
     * @return a true response if successful, a false response if failed.
     */
    bool fromXML(const QString &xml);

};

#endif // LIBKIS_TRANSFORMMASK_H


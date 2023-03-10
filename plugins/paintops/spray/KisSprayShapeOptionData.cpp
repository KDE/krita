/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSprayShapeOptionData.h"

#include "kis_properties_configuration.h"


const QString SPRAYSHAPE_ENABLED = "SprayShape/enabled";
const QString SPRAYSHAPE_SHAPE = "SprayShape/shape";
const QString SPRAYSHAPE_PROPORTIONAL = "SprayShape/proportional";
const QString SPRAYSHAPE_WIDTH = "SprayShape/width";
const QString SPRAYSHAPE_HEIGHT = "SprayShape/height";
const QString SPRAYSHAPE_IMAGE_URL = "SprayShape/imageUrl";
const QString SPRAYSHAPE_USE_ASPECT = "SprayShape/useAspect";


bool KisSprayShapeOptionData::read(const KisPropertiesConfiguration *settings)
{
	enabled = settings->getBool(SPRAYSHAPE_ENABLED, true);
	
	
	size.setWidth(settings->getInt(SPRAYSHAPE_WIDTH));
	size.setHeight(settings->getInt(SPRAYSHAPE_HEIGHT));

	proportional = settings->getBool(SPRAYSHAPE_PROPORTIONAL);

	// particle type size
	shape = settings->getInt(SPRAYSHAPE_SHAPE);
	// you have to check if the image is null in client
	QString url = settings->getString(SPRAYSHAPE_IMAGE_URL);
	if (url.isEmpty()) {
		image = QImage();
	}
	else {
		image = QImage(url);
	}
	imageUrl = url;

    return true;
}

void KisSprayShapeOptionData::write(KisPropertiesConfiguration *settings) const
{
	// settings->setProperty(SHAPE_DYNAMICS_DRAWING_ANGLE_WEIGHT, followDrawingAngleWeight);
	settings->setProperty(SPRAYSHAPE_ENABLED, enabled);

	settings->setProperty(SPRAYSHAPE_WIDTH, size.width());
	settings->setProperty(SPRAYSHAPE_HEIGHT, size.height());
	settings->setProperty(SPRAYSHAPE_PROPORTIONAL, proportional);

	// particle type size
	settings->setProperty(SPRAYSHAPE_SHAPE, shape);
	// you have to check if the image is null in client
    settings->setProperty(SPRAYSHAPE_IMAGE_URL, imageUrl);
}

QSize KisSprayShapeOptionData::effectiveSize(int diameter, qreal scale) const
{
    return !proportional ? size : size * diameter * scale / 100.0;
}

/*
 *  Copyright (c) 2004, Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <stdlib.h>

#include <qobject.h>
#include <qstrlist.h>
#include <qdict.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <klocale.h>
#include <kdebug.h>

#include "kis_abstract_capability.h"
#include "kis_capability_mediator.h"

#include "kis_image_type.h"

KisCapabilityMediator::KisCapabilityMediator() 
{
	initImageTypes();
}

KisCapabilityMediator::~KisCapabilityMediator() 
{
}


QStringList KisCapabilityMediator::getImageTypes() {

	return QStringList(m_imageTypes.keys());
}

KisImageTypeSP KisCapabilityMediator::getImageType(const QString & imageType) {
	if (!m_imageTypes.contains(imageType)) {
		kdDebug() << "Imagetype " << imageType << " does not exist. Aborting Krita.\n";
		abort();
	}

	return m_imageTypes[imageType];
}

void KisCapabilityMediator::initImageTypes() {
	// XXX: this could conceivably be read from a configuration file,
	// and the types could be plugins.
	m_imageTypes["RGBA"] = new KisImageType(i18n("RGBA"), i18n("RGB + Alpha"));
	m_imageTypes["RGB"] = new KisImageType(i18n("RGB"), i18n("RGB"));
	m_imageTypes["CMYKA"] = new KisImageType(i18n("CMYKA"), i18n("CMYK + Alpha"));
	m_imageTypes["CMYK"] = new KisImageType(i18n("CMYK"), i18n("CMYK"));
	m_imageTypes["WET"] = new KisImageType(i18n("Watercolor"), i18n("Watercolor"));
}


#include "kis_capability_mediator.moc"

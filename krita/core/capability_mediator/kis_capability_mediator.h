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

#if !defined KIS_CAPABILITY_MEDIATOR_H
#define KIS_CAPABILITY_MEDIATOR_H

#include <qobject.h>
#include <qmap.h> // Replace with std::map once I get back to the docs.


class QStringList;

#include "kis_image_type.h"

/**
   The CapabilityMediator is the root of the tree that defines
   Krita's capabilities. It stores references to the singletons
   that keep the composite ops, the paint ops, the image types --
   all the stuff traditonally kept in enums.

   This class could be compared with a database, I guess.

   Make this a template for each capability type. Once central
   instance that contains the image types; the image types contain
   colour strategies, the colour strategies composite ops. The
   iamge types also contain paint ops, because not every paint op
   is suitable for every image type.
*/
class KisCapabilityMediator : public QObject {
	Q_OBJECT

public:
	KisCapabilityMediator();
	virtual ~KisCapabilityMediator();

public:

	virtual QStringList getImageTypes();
	virtual KisImageTypeSP getImageType(const QString & imageType);

private:
	void initImageTypes();

	typedef QMap<QString, KisImageTypeSP> ImageTypeMap;
	ImageTypeMap m_imageTypes;
};

#endif // KIS_CAPABILITY_MEDIATOR_H


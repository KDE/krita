/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#if !defined KIS_IMAGE_BUILDER_H_
#define KIS_IMAGE_BUILDER_H_

#include <qobject.h>
#include "kis_types.h"
#include "kis_global.h"

class QString;
class KURL;
class KisDoc;
class KisImageBuilderPriv;

enum KisImageBuilder_Result {
	KisImageBuilder_RESULT_FAILURE = -400,
	KisImageBuilder_RESULT_NOT_EXIST = -300,
	KisImageBuilder_RESULT_NOT_LOCAL = -200,
	KisImageBuilder_RESULT_BAD_FETCH = -100,
	KisImageBuilder_RESULT_OK = 0,
	KisImageBuilder_RESULT_EMPTY = 100,
	KisImageBuilder_RESULT_NO_URI = 200
};

/**
 * Build a KisImage representation of a file image.
 */
class KisImageBuilder {
public:
	KisImageBuilder(KisDoc *doc, const QString& filename);
	KisImageBuilder(KisDoc *doc, const KURL& uri);
	virtual ~KisImageBuilder();

public:
	KisImageBuilder_Result buildImage();
	KisImageSP image();

private:
	KisImageBuilder(const KisImageBuilder&);
	KisImageBuilder& operator=(const KisImageBuilder&);
	void init(KisDoc *doc, const KURL& uri);

private:
	KisImageBuilderPriv *m_members;
};

#endif // KIS_IMAGE_BUILDER_H_


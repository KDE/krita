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
#if !defined KIS_MAGICK_H_
#define KIS_MAGICK_H_

#include "kis_types.h"
#include "kis_global.h"

class QString;
class KURL;

/**
 * Factory to create KisImage from 
 */
class KisImageIO : public QObject {
public:
	KisImageIO(const QString& filename, const QString& format);
	KisImageIO(const KURL& uri, const QString& format);

public:
	virtual KisImageSP image() = 0;

signals:
	void result(bool success);
};

#endif // KIS_MAGICK_H_


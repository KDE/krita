/* 
 * This file is part of the KDE project
 * 
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _KIS_FILEFORMAT_H_
#define _KIS_FILEFORMAT_H_

#include <ksharedptr.h>

class QString;
class QStringList;
class QWidget;

/**
 * A file format defines a particular graphics file format with some
 * capabilities.
 */
class KisFileFormat : public KShared {


public:

	KisFileFormat() {};
	virtual ~KisFileFormat();


public:

        virtual bool read() = 0;

        virtual bool write(KisPaintDeviceSP paintDevice) = 0;

        virtual bool supportsMultiLayer() { return false; };

        virtual QStringList getExtensions() = 0;

        virtual QString * getDescription() = 0;

        virtual QWidget * createConfigWidget(QWidget * parent) = 0;

	/**
	 * This method is used to cooperatively resolve conflicts.
	 * For instance, ImageMagick can handle jpeg, but cannot easily handle
	 * EXIF data (XXX: is that true? Doesn't matter, this is an example),
	 * kimgio can handle jpeg but only create a QImage from it, dropping
	 * all other data, like profiles and exif data. And a hypothetical 
	 * Krit jpeg plugin would be able to handle the complete spec and
	 * make sure that a roundtrip import/export where only the exif annotations
	 * are changes doesn't drop any other information and doesn't degrade the image.
	 */
	virtual bool isBetter(const KisFileFormat *) = 0;


private:


};

#endif // _KIS_FILEFORMAT_H_

/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _JP2_CONVERTER_H_
#define _JP2_CONVERTER_H_

#include <stdio.h>

#include <QObject>
#include <QVector>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_annotation.h"
#include <KisImportExportErrorCode.h>

class KisDocument;

struct JP2ConvertOptions {
	int rate;
	int numberresolution;
};

class JP2Converter: public QObject {
Q_OBJECT
public:
	JP2Converter(KisDocument *doc);
	virtual ~JP2Converter();
public:
	KisImportExportErrorCode buildImage(const QString &filename);
	KisImportExportErrorCode buildFile(const QString &filename,
			KisPaintLayerSP layer, const JP2ConvertOptions &options);
	/**
	 * Retrieve the constructed image
	 */
	KisImageWSP image();
	void addErrorString(const std::string&  str);
	void addWarningString(const std::string&  str);
	void addInfoString(const std::string&  str);

private:
	KisImportExportErrorCode decode(const QString &filename);
	int infile_format(const char *fname);
public Q_SLOTS:
	virtual void cancel();
private:
	KisImageSP m_image;
	KisDocument *m_doc;
	bool m_stop;
	std::string err;
	std::string warn;
};

#endif

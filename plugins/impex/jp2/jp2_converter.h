/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
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

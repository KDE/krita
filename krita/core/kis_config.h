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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_CONFIG_H_
#define KIS_CONFIG_H_

#include "kis_global.h"

class KisConfig {
public:
	KisConfig();
	~KisConfig();

	Q_INT32 maxImgWidth() const;
	Q_INT32 defImgWidth() const;
	void defImgWidth(Q_INT32 width);

	Q_INT32 maxImgHeight() const;
	Q_INT32 defImgHeight() const;
	void defImgHeight(Q_INT32 height);

	enumCursorStyle defCursorStyle() const;
	void defCursorStyle(enumCursorStyle style);

	enumDockerStyle dockerStyle() const;
	void setDockerStyle( enumDockerStyle style );

	QString monitorProfile() const;
	void setMonitorProfile(QString monitorProfile);

	QString workingColorSpace() const;
	void setWorkingColorSpace(QString workingColorSpace);

	QString importProfile() const;
	void setImportProfile(QString importProfile);

	QString printerColorSpace() const;
	void setPrinterColorSpace(QString printerColorSpace);

	QString printerProfile() const;
	void setPrinterProfile(QString printerProfile);

	bool useBlackPointCompensation() const;
	void setUseBlackPointCompensation(bool useBlackPointCompensation);

	bool dither8Bit() const;
	void setDither8Bit(bool dither8Bit);

	bool askProfileOnOpen() const;
	void setAskProfileOnOpen(bool askProfileOnOpen);

	bool askProfileOnPaste() const;
	void setAskProfileOnPaste(bool askProfileOnPaste);

	bool applyMonitorProfileOnCopy() const;
	void setApplyMonitorProfileOnCopy(bool applyMonitorProfileOnCopy);

	Q_INT32 renderIntent();
	void setRenderIntent(Q_INT32 renderIntent);

	Q_INT32 maxNumberOfThreads();
	void setMaxNumberOfThreads(Q_INT32 numberOfThreads);

private:
	KisConfig(const KisConfig&);
	KisConfig& operator=(const KisConfig&);

private:
	mutable KConfig *m_cfg;
};

#endif // KIS_CONFIG_H_

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
#if !defined KIS_CONFIG_H_
#define KIS_CONFIG_H_

#include <map>
#include "kis_global.h"

class KisConfig {
public:
	KisConfig();
	~KisConfig();

	Q_INT32 maxImgWidth() const;
	Q_INT32 defImgWidth() const;
	Q_INT32 maxImgHeight() const;
	Q_INT32 defImgHeight() const;
	Q_INT32 maxLayerWidth() const;
	Q_INT32 defLayerWidth() const;
	Q_INT32 maxLayerHeight() const;
	Q_INT32 defLayerHeight() const;
	enumImgType defImgType() const;

	void defImgWidth(Q_INT32 width);
	void defImgHeight(Q_INT32 height);
	void defLayerWidth(Q_INT32 width);
	void defLayerHeight(Q_INT32 height);
	void defImgType(enumImgType imgType);

private:
	KisConfig(const KisConfig&);
	KisConfig& operator=(const KisConfig&);

	QString imgTypeName(enumImgType imgType) const;
	enumImgType imgType(const QString& name) const;
	void setupImgTypeNames() const;

private:
	mutable KConfig *m_cfg;
	static std::map<enumImgType, QString> m_imgTypeName;
};

#endif // KIS_CONFIG_H_


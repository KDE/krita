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

#if !defined KIS_ABSTRACT_CAPABILITY_H
#define KIS_ABSTRACT_CAPABILITY_H

#include <qstring.h>

/**
   AbstractCapability is the base class from which specific
   Krita capabilities, like composite ops or paint ops are
   derived.

   These operations, because they may be needed in the UI,
   all have a label and a description.

*/
class KisAbstractCapability {

public:
	KisAbstractCapability();
	KisAbstractCapability(const QString & label,
			      const QString & desctription);
	virtual ~KisAbstractCapability();


	QString label() const;
	void setLabel(const QString & label);
	QString description() const;
	void setDescription(const QString & description);
	
private:

	QString m_label;
	QString m_description;

};

#endif // KIS_ABSTRACT_CAPABILITY_H


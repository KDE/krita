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
#include <qdict.h>

class KisAbstractCapability;
class QStrList;


/**
   The CapabilityMediator is the root of the tree that defines
   Krita's capabilities. It stores references to the singletons
   that keep the composite ops, the paint ops, the image types --
   all the stuff traditonally kept in enums.
*/
class KisCapabilityMediator : public QObject {
	Q_OBJECT

public:
	KisCapabilityMediator();
	virtual ~KisCapabilityMediator();

public:
	/**
	 */
	virtual void registerCapability(const QString & name, 
					const KisAbstractCapability capability) = 0;
	/**
	   Return a QStrList filled with the names of the capabilities that satisfy the
	   specified filter. (XXX: the filter needs perhaps be a little more sophisticated, 
	   or something simpler, like a path. Or perhaps even an URI.)
	 */
	virtual QStrList* capabilities(const QString & filter) const = 0;

	/**
	   Retrieve the capabilitye with the specified name.
	*/
	virtual KisAbstractCapability * getCapability(const QString & name) const = 0;
private:
	
	QDict<KisAbstractCapability> * m_capabilities;
};

#endif // KIS_CAPABILITY_MEDIATOR_H


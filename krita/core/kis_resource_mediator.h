/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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
#if !defined KIS_RESOURCE_MEDIATOR_H_
#define KIS_RESOURCE_MEDIATOR_H_

#include <qobject.h>
#include <qmap.h>
#include <qwidget.h>

class KoIconItem;
class KisBrush;
class KisImagePipeBrush;
class KisPattern;
class KisItemChooser;
class KisIconItem;
class KisResource;
class KisResourceServer;

#define	MEDIATE_BRUSHES 1
#define MEDIATE_PATTERNS 2

class KisResourceMediator : public QObject {
	Q_OBJECT
	typedef QObject super;

public:
	KisResourceMediator(Q_INT32 mediateOn,
			    KisResourceServer *rserver,
			    const QString& chooserCaption,
			    QWidget *chooserParent = 0,
			    const char *chooserName = 0,
			    QObject *parent = 0,
			    const char *name = 0);
	virtual ~KisResourceMediator();

public:
	KisResource *currentResource() const;
	KisIconItem *itemFor(KisResource *r) const;
	KisResource *resourceFor(KoIconItem *item) const;
	KisResource *resourceFor(KisIconItem *item) const;
	QWidget *chooserWidget() const;

signals:
	void activatedResource(KisResource *r);
	void addedResource(KisResource *r);

private slots:
	void setActiveItem(KoIconItem *item);
	void resourceServerLoadedResource(KisResource *resource);

private:
	KisItemChooser *m_chooser;
	QMap<KisResource*, KisIconItem*> m_items;
	KoIconItem *m_activeItem;
};

#endif // KIS_RESOURCE_MEDIATOR_H_


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

#ifndef KIS_CANVAS_SUBJECT_H_
#define KIS_CANVAS_SUBJECT_H_

#include <qstring.h>
#include <koColor.h>
#include "kis_types.h"

class KoDocument;
class KisBrush;
class KisCanvasControllerInterface;
class KisCanvasObserver;
class KisGradient;
class KisPattern;
class KisToolControllerInterface;
class KisUndoAdapter;
class KisProgressDisplayInterface;

class KisCanvasSubject {
public:
	KisCanvasSubject();
	virtual ~KisCanvasSubject();

public:
	virtual void attach(KisCanvasObserver *observer) = 0;
	virtual void detach(KisCanvasObserver *observer) = 0;
	virtual void notify() = 0;
	virtual KisImageSP currentImg() const = 0;
	virtual QString currentImgName() const = 0;
	virtual KoColor bgColor() const = 0;
	virtual void setBGColor(const KoColor& c) = 0;
	virtual KoColor fgColor() const = 0;
	virtual void setFGColor(const KoColor& c) = 0;
	virtual KisBrush *currentBrush() const = 0;
	virtual KisPattern *currentPattern() const = 0;
	virtual KisGradient *currentGradient() const = 0;
	virtual double zoomFactor() const = 0;
	virtual KisUndoAdapter *undoAdapter() const = 0;
	virtual KisCanvasControllerInterface *canvasController() const = 0;
	virtual KisToolControllerInterface *toolController() const = 0;
	virtual KoDocument *document() const = 0;
	virtual KisProgressDisplayInterface *progressDisplay() const = 0;
	virtual KisFilterSP filterGet(const QString& name) = 0;
	virtual QStringList filterList() = 0;

private:
	KisCanvasSubject(const KisCanvasSubject&);
	KisCanvasSubject& operator=(const KisCanvasSubject&);
};

#endif // KIS_CANVAS_SUBJECT_H_


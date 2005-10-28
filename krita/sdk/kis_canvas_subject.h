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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_CANVAS_SUBJECT_H_
#define KIS_CANVAS_SUBJECT_H_

#include "kis_types.h"
#include "kis_id.h"

class KisDoc;
class KisBrush;
class KisCanvasController;
class KisCanvasObserver;
class KisGradient;
class KisPattern;
class KisPaintOpFactory;
class KisToolControllerInterface;
class KisUndoAdapter;
class KisProgressDisplayInterface;
class KisSelectionManager;
class QWidget;
class QCursor;
class KisColor;
class KoPaletteManager;
class KisProfile;

/**
 * The canvas subject is the interface implemented by classes that perform the
 * "controller" role in the model-view-controller paradigm.
 */
class KisCanvasSubject {

public:
    KisCanvasSubject() {};
    virtual ~KisCanvasSubject() {};

public:

    /**
     * From now on, the observer will be notified of changes in
     * brush, foreground color, background color, pattern, gradient
     * and paintop
     */
    virtual void attach(KisCanvasObserver *observer) = 0;
    virtual void detach(KisCanvasObserver *observer) = 0;
    
    virtual void notifyObservers() = 0;
    
    virtual KisImageSP currentImg() const = 0;
    
    virtual KisColor bgColor() const = 0;
    virtual void setBGColor(const KisColor& c) = 0;
    
    virtual KisColor fgColor() const = 0;
    virtual void setFGColor(const KisColor& c) = 0;
    
    virtual float HDRExposure() const = 0;
    virtual void setHDRExposure(float exposure) = 0;
    
    virtual KisBrush *currentBrush() const = 0;

    virtual KisPattern *currentPattern() const = 0;
    
    virtual KisGradient *currentGradient() const = 0;
    
    virtual KisID currentPaintop() const = 0;
    
    virtual double zoomFactor() const = 0;
    
    virtual KisUndoAdapter *undoAdapter() const = 0;
    
    virtual KisCanvasController *canvasController() const = 0;
    
    virtual KisToolControllerInterface *toolController() const = 0;
    
    virtual KisDoc * document() const = 0;
    
    virtual KisProgressDisplayInterface *progressDisplay() const = 0;
    
    virtual KisSelectionManager * selectionManager() = 0;

    virtual KoPaletteManager * paletteManager() = 0;

    /**
     * Get the profile that this view uses to display itself on
     * he monitor.
     */
    virtual KisProfile *  monitorProfile() = 0;


private:
    KisCanvasSubject(const KisCanvasSubject&);
    KisCanvasSubject& operator=(const KisCanvasSubject&);
};

#endif // KIS_CANVAS_SUBJECT_H_


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
#include "KoID.h"

class KisDoc;
class KisBrush;
class KisCanvasController;
class KisCanvasObserver;
class KisGradient;
class KisPattern;
class KisPaintOpFactory;
class KisToolControllerInterface;
class KisUndoAdapter;
class KisPerspectiveGridManager;
class KisProgressDisplayInterface;
class KisSelectionManager;
class QWidget;
class QCursor;
class KoColor;
class KoColorProfile;
class KisPaintOpSettings;

/**
 * KisCanvasSubject is part of the Observer pattern. Classes implementing KisCanvasObserver
 * that have attached themselves to a class implementing this interface will be notified
 * whenever any of the methods defined in this class will return something different.
 *
 * Historical notes: This class has grown a bit an now seems to be the abstract definition
 * of the View part in the Model-View-Controller pattern. We need to fix this!
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

    /**
     * From now on, the specified observer will no longer be informed
     * of changes.
     */
    virtual void detach(KisCanvasObserver *observer) = 0;
    
    /**
     * Calling this method will notify all observers. Do not call this
     * method from the Update method of a KisCanvasObserver!
     */
    virtual void notifyObservers() = 0;
    
    /**
     * @return the image that is currently active.
     */
    virtual KisImageSP currentImg() const = 0;
    
    /**
     * @return the background color
     */
    virtual KoColor bgColor() const = 0;

    /**
     * Set the background color. This should cause all observers to be notified. Do not call from KisCanvasObserver::update()!
     *
     * @param c the new background color
     */
    virtual void setBGColor(const KoColor& c) = 0;
    
    /**
     * @return the currently set foreground or painting color
     */
    virtual KoColor fgColor() const = 0;

    /**
     * Set the foreground or painting color. This should cause all observers to be notified. Do not call from KisCanvasObserver::update()!
     *
     * @param c the new foreground color
     */
    virtual void setFGColor(const KoColor& c) = 0;
    
    /**
     * @return the exposure value. This determines which exposure of multiple exposure or HDR images will be displayed
     */
    virtual float HDRExposure() const = 0;

    /** 
     * Set the exposure value. This determines which exposure of multiple exposure or HDR images will be displayed. All observers should be notified.
     *
     * @param exposure the new exposure value
     */
    virtual void setHDRExposure(float exposure) = 0;
    
    /**
     * @return the current brush object.
     */
    virtual KisBrush *currentBrush() const = 0;

    /**
     * @return the current pattern object.
     */
    virtual KisPattern *currentPattern() const = 0;
    
    /**
     * @return the current gradient object
     */
    virtual KisGradient *currentGradient() const = 0;
    
    /**
     * @return the identification of the current paintop object, not the paintop object itself.
     */
    virtual KoID currentPaintop() const = 0;

    /**
     * @return the settings for the current paintop object, or 0 if there are no options set.
     */
    virtual const KisPaintOpSettings *currentPaintopSettings() const = 0;

    /**
     * @return the currently applied zoom factor. XXX: Shouldn't this be in the canvas controller?
     */
    virtual double zoomFactor() const = 0;
    
    /**
     * retrieve the undo adapter -- there is one undo adapter
     * per document, and it collects all transactions
     */
    virtual KisUndoAdapter *undoAdapter() const = 0;
    
    /**
     * @return the interface to the canvas widget
     */
    virtual KisCanvasController *canvasController() const = 0;
    
    /// XXX: Remove this method
    virtual KisToolControllerInterface *toolController() const = 0;
    
    /// XXX: Remove this method
    virtual KisDoc * document() const = 0;
    
    /// XXX: Remove this method
    virtual KisProgressDisplayInterface *progressDisplay() const = 0;
    
    /// XXX: Remove this method
    virtual KisSelectionManager * selectionManager() = 0;

    /**
     * Get the profile that this view uses to display itself on
     * he monitor.
     */
    virtual KoColorProfile *  monitorProfile() = 0;

    /**
     * Get the perspective grid manager.
     */
    virtual KisPerspectiveGridManager* perspectiveGridManager() = 0;

private:
    KisCanvasSubject(const KisCanvasSubject&);
    KisCanvasSubject& operator=(const KisCanvasSubject&);
};

#endif // KIS_CANVAS_SUBJECT_H_


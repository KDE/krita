/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_PAINTOP_SETTINGS_H_
#define KIS_PAINTOP_SETTINGS_H_

#include "opengl/kis_opengl.h"

#include "kis_types.h"
#include "krita_export.h"

#include <QImage>

#include "kis_shared.h"
#include "kis_properties_configuration.h"
#include "kis_paint_information.h"


class KoPointerEvent;
class KoViewConverter;
class KisPaintOpSettingsWidget;

/**
 * This class is used to cache the settings for a paintop
 * between two creations. There is one KisPaintOpSettings per input device (mouse, tablet,
 * etc...).
 *
 * The settings may be stored in a preset or a recorded brush stroke. Note that if your
 * paintop's settings subclass has data that is not stored as a property, that data is not
 * saved and restored.
 */
class KRITAIMAGE_EXPORT KisPaintOpSettings : public KisPropertiesConfiguration, public KisShared
{

public:

    KisPaintOpSettings();

    virtual ~KisPaintOpSettings();

    /**
     *
     */
    virtual void setOptionsWidget(KisPaintOpSettingsWidget* widget);

    /**
     * This function is called by a tool when the mouse is pressed. It's useful if
     * the paintop needs mouse interaction for instance in the case of the duplicate op.
     * If the tool is supposed to ignore the event, the paint op should return false
     * and if the tool is supposed to use the event, return true.
     */
    virtual bool mousePressEvent(const KisPaintInformation &pos, Qt::KeyboardModifiers modifiers);

    /**
     * Clone the current settings object. Override this if your settings instance doesn't
     * store everything as properties.
     */
    virtual KisPaintOpSettingsSP clone() const;

    /**
     * Override this function if your paintop is interested in which
     * node is currently active.
     */
    virtual void setNode(KisNodeSP node);

    /**
     * @return the node the paintop is working on.
     */
    KisNodeSP node() const;

    /**
     * Call this function when the paint op is selected or the tool is activated
     */
    virtual void activate();

    /**
     * XXX: Remove this after 2.0, when the paint operation (incremental/non incremental) will
     *      be completely handled in the paintop, not in the tool. This is a filthy hack to move
     *      the option to the right place, at least.
     * @return true if we paint incrementally, false if we paint like Photoshop. By default, paintops
     *      do not support non-incremental.
     */
    virtual bool paintIncremental() {
        return true;
    }

    /**
     * Whether this paintop wants to deposit paint even when not moving, i.e. the
     * tool needs to activate its timer.
     */
    virtual bool isAirbrushing() const {
        return false;
    }

    /**
    * If this paintop deposit the paint even when not moving, the tool needs to know the rate of it in miliseconds
    */
    virtual int rate() const{
        return 100;
    }

    /**
     * This enum defines the current mode for painting an outline.
     */
    enum OutlineMode {
        CursorIsOutline = 1, ///< When this mode is set, an outline is painted around the cursor
        CursorIsNotOutline = 2 ///< Currently, this mode means that there is no outline active. It used to mean using of QImage-based outlines (e.g. for duplicate tool) but it was not implemented.
    };

    /**
     * Returns the brush outline in pixel coordinates. Tool is responsible for conversion into view coordinates.
     * Outline mode has to be passed to the paintop which builds the outline as some paintops have to paint outline
     * always like duplicate paintop indicating the duplicate position
     */
    virtual QPainterPath brushOutline(const QPointF& pos, OutlineMode mode, qreal scale = 1.0, qreal rotation = 0.0) const;

    /**
    * Useful for simple elliptical brush outline.
    */
    QPainterPath ellipseOutline(qreal width, qreal height, qreal scale, qreal rotation) const;

    /**
     * The behaviour might be different per paintop. Most of the time
     * the brush diameter is increased by x pixels, y ignored
     *
     * @param x is add to the the diameter or radius (according the paintop)
     *  It might be also negative, to decrease the value of the brush diameter/radius.
     *  x is in pixels
     * @param y is unused, it supposed to be used to change some different attribute
     * of the brush like softness or density
     */
    virtual void changePaintOpSize(qreal x, qreal y);

    /**
     * @return The width and the height of the brush mask/dab in pixels
     */
    virtual QSizeF paintOpSize() const;

    /**
     * @return filename of the 3D brush model, empty if no brush is set
     */
    virtual QString modelName() const;

     /**
     * Set filename of 3D brush model. By default no brush is set
     */
    void setModelName(const QString & modelName);

    /// Check if the settings are valid, setting might be invalid through missing brushes etc
    /// Overwrite if the settings of a paintop can be invalid
    /// @return state of the settings, default implementation is true
    virtual bool isValid();

    /// Check if the settings are loadable, that might the the case if we can fallback to something
    /// Overwrite if the settings can do some kind of fallback
    /// @return loadable state of the settings, by default implementation return the same as isValid()
    virtual bool isLoadable();

    void setCanvasRotation(qreal angle);
    qreal canvasRotation() const;

protected:
     /**
     * @return the option widget of the paintop (can be 0 is no option widgets is set)
     */
    KisPaintOpSettingsWidget* optionsWidget() const;

private:

    struct Private;
    Private* const d;

};

#endif

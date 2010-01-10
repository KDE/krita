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
     * If the tool is supposed to ignore the event, the paint op should call e->accept();
     * and if the tool is supposed to use the event e->ignore(); should be called.
     */
    virtual void mousePressEvent(KoPointerEvent *e);

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
     * @return a sample stroke that fits in @param size.
     */
    QImage sampleStroke(const QSize& size);

    /**
     * This enum defines the current mode for painting an outline.
     */
    enum OutlineMode {
        CURSOR_IS_OUTLINE = 1, ///< When this mode is set, then the outline is supposed to paint an outline around the cursor
        CURSOR_ISNT_OUTLINE = 2 ///< When this mode is set, then the outline is no supposed to paint an outline for the cursor (useful for instance in the duplicate op to show the source)
    };
    /**
     * @return the rectangle covered by the current brush (or the previous brush??? and what about pressure???)
     * based on the given position, in XXX (image or view???) coordinates.
     *
     * XXX: the function name is very misleading, since this function doesn't do any painting! Rename
     * to brushOutlineRect, and perhaps just return the brushSize and let the caller handle the x,y
     * location. If one wants to use QImage, then one use something else
     */
    virtual QRectF paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const;

    /**
     * This function allow the paintop to draw an outline at a given position.
     *
     * XXX: It would be _much_ better to pass return a QImage, instead of pass a painter and
     * a KoViewConverter (which is _not_ a class that that should be referenced in krita/image (we could make it return a KisPaintopOutlineDrawer whose implementation can be in krita/ui).
     * And we need a lot of caching here, since no matter what we do, it is utterly slow, especially
     * when using a tablet. How does XXX works with the duplicate op ? List of images ? With a center ?
     */
    virtual void paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const;

    virtual void changePaintOpSize(qreal x, qreal y);
    
#if defined(HAVE_OPENGL)
    virtual QString modelName() const;
#endif

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

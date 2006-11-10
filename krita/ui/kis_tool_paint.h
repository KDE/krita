/*
 *  Copyright (c) 2003 Boudewijn Rempt
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

#ifndef KIS_TOOL_PAINT_H_
#define KIS_TOOL_PAINT_H_

#include <QCursor>
#include <QLayout>
#include <QLabel>
#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>

#include <KoCanvasResourceProvider.h>
#include <KoTool.h>

#include <krita_export.h>

#include <kis_types.h>
#include <kis_image.h>

#include <kis_brush.h>
#include <kis_gradient.h>
#include <kis_pattern.h>

#include "kis_tool.h"
#include "KoCompositeOp.h"

class QCheckBox;
class QEvent;
class QKeyEvent;
class QComboBox;
class QPaintEvent;
class QRect;
class QGridLayout;
class QLabel;

class KDialog;

class KoCanvasBase;

class KisCmbComposite;
class KisIntSpinbox;
class KisPaintOpSettings;

enum enumBrushMode {
    PAINT,
    PAINT_STYLUS,
    ERASE,
    ERASE_STYLUS,
    HOVER
};

class KRITAUI_EXPORT KisToolPaint
    : public KoTool
{

    Q_OBJECT

public:
    KisToolPaint(KoCanvasBase * canvas);
    virtual ~KisToolPaint();

// KoTool Implementation.

public slots:

    virtual void activate(bool temporary = false);
    virtual void deactivate();
    virtual void resourceChanged( const KoCanvasResource & res );

public:

    virtual void paint(QPainter& gc, KoViewConverter &converter);

    virtual void mouseReleaseEvent( KoPointerEvent *event );

protected:

    /// @return the image wrapped in the dummy shape in the shape
    /// manager. XXX: This is probably wrong!
    KisImageSP image() const;

    /// Call this to set the document modified
    void notifyModified() const;

    /// Add the tool-specific layout to the default option widget layout.
    void addOptionWidgetLayout(QLayout *layout);

    /// Add a widget and a label to the current option widget layout.
    virtual void addOptionWidgetOption(QWidget *control, QWidget *label = 0);

    virtual void createOptionWidget();

private:

    // XXX: Call this when the layer changes (this used to be called in KisCanvasObserver::update)
    void updateCompositeOpComboBox();

private slots:

    void slotPopupQuickHelp();
    void slotSetOpacity(int opacityPerCent);
    void slotSetCompositeMode(const KoCompositeOp* compositeOp);

protected:

    QRect m_dirtyRect;
    quint8 m_opacity;
    const KoCompositeOp * m_compositeOp;
    bool m_paintOutline;

    KisImageSP m_currentImage;

    // From the canvas resources
    KisBrush * m_currentBrush;
    KisPattern * m_currentPattern;
    KisGradient * m_currentGradient;
    KoColor m_currentFgColor;
    KoColor m_currentBgColor;
    KoID m_currentPaintOp;
    KisPaintOpSettings * m_currentPaintOpSettings;
    KisLayerSP m_currentLayer;
    float m_currentExposure;

private:

    QCursor m_cursor;

    QGridLayout *m_optionWidgetLayout;

    QLabel *m_lbOpacity;
    KisIntSpinbox *m_slOpacity;
    QLabel *m_lbComposite;
    KisCmbComposite *m_cmbComposite;
};

#endif // KIS_TOOL_PAINT_H_


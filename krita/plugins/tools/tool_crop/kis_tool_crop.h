/*
 *  kis_tool_crop.h - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_CROP_H_
#define KIS_TOOL_CROP_H_

#include <qpoint.h>
#include <qregion.h>
#include <kis_tool_non_paint.h>
#include <kis_tool_factory.h>

class QRect;
class QCursor;
class WdgToolCrop;

/**
 * Crop tool
 *
 * TODO: - crop from selection -- i.e, set crop outline to the exact bounds of the selection.
 *       - (when moving to Qt 4: replace rectangle with  darker, dimmer overlay layer
 *         like we have for selections right now)
 */
class KisToolCrop : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:

    KisToolCrop();
    virtual ~KisToolCrop();

    virtual void update(KisCanvasSubject *subject);

    virtual QWidget* createOptionWidget(QWidget* parent);
    virtual QWidget* optionWidget();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_TRANSFORM; }
    virtual Q_UINT32 priority() { return 1; }
    virtual void paint(KisCanvasPainter& gc);
    virtual void paint(KisCanvasPainter& gc, const QRect& rc);
    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);
    virtual void doubleClick(KisDoubleClickEvent *);

public slots:

    virtual void activate();
    virtual void deactivate();

private:

    void clearRect();
    QRegion handles(QRect rect);
    void paintOutlineWithHandles();
    void paintOutlineWithHandles(KisCanvasPainter& gc, const QRect& rc);
    Q_INT32 mouseOnHandle (const QPoint currentViewPoint);
    void setMoveResizeCursor (Q_INT32 handle);
    void validateSelection(bool updateratio = true);
    void setOptionWidgetX(Q_INT32 x);
    void setOptionWidgetY(Q_INT32 y);
    void setOptionWidgetWidth(Q_INT32 x);
    void setOptionWidgetHeight(Q_INT32 y);
    void setOptionWidgetRatio(double ratio);

private slots:

    void crop();
    void setCropX(int x);
    void setCropY(int y);
    void setCropWidth(int x);
    void setCropHeight(int y);
    void setRatio(double ratio);

    inline QRect realRectCrop() { QRect r = m_rectCrop; r.setSize(r.size() - QSize(1,1)); return r; }
    
private:
    void updateWidgetValues(bool updateratio = true);
    KisCanvasSubject *m_subject;
    QRect m_rectCrop; // Is the coordinate of the outline rect and not of the region to crop (to get the region to crop you need to remove 1 to width and height
//     QPoint m_startPos;
//     QPoint m_endPos;
    bool m_selecting;
    QPoint m_dragStart;
    QPoint m_dragStop;

    WdgToolCrop* m_optWidget;

    Q_INT32 m_handleSize;
    QRegion m_handlesRegion;
    bool m_haveCropSelection;
    Q_INT32 m_dx, m_dy;
    Q_INT32 m_mouseOnHandleType;
    QCursor m_cropCursor;

    enum handleType
    {
        None = 0,
        UpperLeft = 1,
        UpperRight = 2,
        LowerLeft = 3,
        LowerRight = 4,
        Upper = 5,
        Lower = 6,
        Left = 7,
        Right = 8,
        Inside = 9
    };
};

class KisToolCropFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolCropFactory() : super() {};
    virtual ~KisToolCropFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t = new KisToolCrop();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("crop", i18n("Crop Tool")); }
};



#endif // KIS_TOOL_CROP_H_


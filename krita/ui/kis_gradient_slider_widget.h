/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <longamp@reallygood.de>
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

#ifndef _KIS_WDG_GRADIENT_SLIDER_H_
#define _KIS_WDG_GRADIENT_SLIDER_H_

#include <qwidget.h>
#include <QMouseEvent>
#include <QPaintEvent>

class KAction;
class KMenu;
class KisAutogradientResource;
class KisGradientSegment;

class KisGradientSliderWidget : public QWidget
{
    Q_OBJECT

public:
    KisGradientSliderWidget(QWidget *parent = 0, const char* name = 0, Qt::WFlags f = 0);

public:
    virtual void paintEvent ( QPaintEvent * );
    void setGradientResource( KisAutogradientResource* agr);
    KisGradientSegment* selectedSegment() { return m_selectedSegment; };

signals:
    void sigSelectedSegment(KisGradientSegment*);
    void sigChangedSegment(KisGradientSegment*);

protected:
    virtual void mousePressEvent( QMouseEvent * e );
    virtual void mouseReleaseEvent ( QMouseEvent * e );
    virtual void mouseMoveEvent( QMouseEvent * e );
    virtual void contextMenuEvent( QContextMenuEvent * e );

private slots:
    void slotSplitSegment();
    void slotDuplicateSegment();
    void slotMirrorSegment();
    void slotRemoveSegment();

private:

    enum {
        NO_DRAG,
        LEFT_DRAG,
        RIGHT_DRAG,
        MIDDLE_DRAG
    };

    enum {
        SPLIT_SEGMENT,
        DUPLICATE_SEGMENT,
        MIRROR_SEGMENT,
        REMOVE_SEGMENT
    };

    KisAutogradientResource* m_autogradientResource;
    KisGradientSegment* m_currentSegment;
    KisGradientSegment* m_selectedSegment;
    KMenu* m_segmentMenu;
    int m_drag;
    KAction *m_removeSegmentAction;
};

#endif

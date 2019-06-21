/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_patches.h"

#include <QApplication>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "kis_canvas2.h"
#include "KoCanvasResourceProvider.h"
#include "kis_display_color_converter.h"


KisColorPatches::KisColorPatches(QString configPrefix, QWidget *parent) :
    KisColorSelectorBase(parent), m_allowColorListChangeGuard(true), m_scrollValue(0), m_configPrefix(configPrefix)
{
    resize(1, 1);
    updateSettings();
}

void KisColorPatches::setColors(QList<KoColor>colors)
{
    if (m_allowColorListChangeGuard) {
        m_colors = colors;

        m_allowColorListChangeGuard=false;

        KisColorPatches* parent = dynamic_cast<KisColorPatches*>(m_parent);
        if (parent) parent->setColors(colors);

        KisColorPatches* popup = dynamic_cast<KisColorPatches*>(m_popup);
        if (popup) popup->setColors(colors);

        m_allowColorListChangeGuard=true;

        update();
    }
}

void KisColorPatches::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    if(m_allowScrolling) {
        if(m_direction == Vertical)
            painter.translate(0, m_scrollValue);
        else
            painter.translate(m_scrollValue, 0);
    }


    int widgetWidth = width();
    int numPatchesInARow = qMax(widgetWidth/m_patchWidth, 1);

    int widgetHeight = height();
    int numPatchesInACol = qMax(widgetHeight/m_patchHeight, 1);

    for(int i = m_buttonList.size(); i < qMin(fieldCount(), m_colors.size() + m_buttonList.size()); i++) {
        int row;
        int col;
        if(m_direction == Vertical) {
            row =  i /numPatchesInARow;
            col = i % numPatchesInARow;
        }
        else {
            row= i % numPatchesInACol;
            col = i / numPatchesInACol;
        }

        QColor qcolor = converter()->toQColor(m_colors.at(i - m_buttonList.size()));

        painter.fillRect(col*m_patchWidth,
                         row*m_patchHeight,
                         m_patchWidth,
                         m_patchHeight,
                         qcolor);
    }

    QWidget::paintEvent(e);

}

void KisColorPatches::wheelEvent(QWheelEvent* event)
{
    m_scrollValue+=event->delta()/2;
    if(m_direction == Vertical) {
        if(m_scrollValue < -1*(heightOfAllPatches()-height()))
            m_scrollValue = -1*(heightOfAllPatches()-height());
    }
    else {
        if(m_scrollValue < -1*(widthOfAllPatches()-width()))
            m_scrollValue = -1*(widthOfAllPatches()-width());
    }
    if(m_scrollValue>0) m_scrollValue=0;


    update();
}

void KisColorPatches::resizeEvent(QResizeEvent* event)
{
    if(size()==QSize(1, 1))
        return;

    QWheelEvent dummyWheelEvent(QPoint(), 0, Qt::NoButton, Qt::NoModifier);
    wheelEvent(&dummyWheelEvent);

    if(parentWidget()==0) {
        // this instance is a popup
        setMinimumWidth(m_patchWidth*(m_patchCount/4));
        setMaximumWidth(minimumWidth());
    }


    if (m_allowScrolling == false && event->oldSize() != event->size()) {
        if(m_direction == Horizontal) {
            setMaximumHeight(heightForWidth(width()));
            setMinimumHeight(heightForWidth(width()));
        }
        else {
            setMaximumWidth(widthForHeight(height()));
            setMinimumWidth(widthForHeight(height()));
        }
    }
}

void KisColorPatches::mouseReleaseEvent(QMouseEvent* event)
{
    KisColorSelectorBase::mouseReleaseEvent(event);
    event->setAccepted(false);
    KisColorSelectorBase::mouseReleaseEvent(event);
    if (event->isAccepted() || !rect().contains(event->pos()))
        return;

    if (!m_canvas) return;

    KoColor color;
    if(colorAt(event->pos(), &color)) {
        if (event->button()==Qt::LeftButton)
            m_canvas->resourceManager()->setForegroundColor(color);
        else if (event->button()==Qt::RightButton)
            m_canvas->resourceManager()->setBackgroundColor(color);
    }
}


void KisColorPatches::mousePressEvent(QMouseEvent *event)
{
    KoColor koColor;
    if(!colorAt(event->pos(), &koColor))
        return;

    KisColorSelectorBase::mousePressEvent(event);
    if(event->isAccepted())
        return;

    updateColorPreview(koColor);

    if (event->button() == Qt::LeftButton)
        m_dragStartPos = event->pos();
}

void KisColorPatches::mouseMoveEvent(QMouseEvent *event)
{
    event->ignore();
    KisColorSelectorBase::mouseMoveEvent(event);
    if(event->isAccepted())
        return;

    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - m_dragStartPos).manhattanLength()
         < QApplication::startDragDistance())
        return;

    KoColor koColor;
    if(!colorAt(m_dragStartPos, &koColor))
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QColor color = converter()->toQColor(koColor);
    mimeData->setColorData(color);
    mimeData->setText(color.name());
    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction);

    event->accept();
}

int KisColorPatches::patchCount() const
{
    return m_patchCount;
}

bool KisColorPatches::colorAt(const QPoint &pos, KoColor *result) const
{
    if(!rect().contains(pos))
        return false;

    int scrollX = m_direction==Horizontal?m_scrollValue:0;
    int scrollY = m_direction==Vertical?m_scrollValue:0;
    int column = (pos.x()-scrollX)/m_patchWidth;
    int row = (pos.y()-scrollY)/m_patchHeight;

    int patchNr;
    if(m_direction == Vertical) {
        int patchesInARow = width()/m_patchWidth;
        patchNr=row*patchesInARow+column;
    }
    else {
        // Vertical
        int patchesInAColumn = height()/m_patchHeight;
        patchNr=column*patchesInAColumn+row;
    }

    patchNr-=m_buttonList.size();

    if(patchNr>=0 && patchNr<m_colors.size()) {
        (*result)=m_colors.at(patchNr);
        return true;
    }
    else return false;
}

void KisColorPatches::setAdditionalButtons(QList<QWidget*> buttonList)
{
    for(int i=0; i<buttonList.size(); i++) {
        buttonList.at(i)->setParent(this);
    }
    m_buttonList = buttonList;
}

void KisColorPatches::updateSettings()
{
    KisColorSelectorBase::updateSettings();

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");

    if(cfg.readEntry(m_configPrefix+"Alignment", false))
        m_direction=Vertical;
    else
        m_direction=Horizontal;
    m_allowScrolling=cfg.readEntry(m_configPrefix+"Scrolling", true);
    m_numCols=cfg.readEntry(m_configPrefix+"NumCols", 1);
    m_numRows=cfg.readEntry(m_configPrefix+"NumRows", 1);
    m_patchCount=cfg.readEntry(m_configPrefix+"Count", 15);
    m_patchWidth=cfg.readEntry(m_configPrefix+"Width", 20);
    m_patchHeight=cfg.readEntry(m_configPrefix+"Height", 20);
    if(m_patchHeight == 0) {
        m_patchHeight = 1;
    }

    if(parentWidget()==0) {
        // this instance is a popup
        m_allowScrolling = false;
        m_direction = Horizontal;
        m_patchWidth*=2;
        m_patchHeight*=2;
    }

    for(int i=0; i<m_buttonList.size(); i++) {
        m_buttonList.at(i)->setGeometry(0, i*m_patchHeight, m_patchWidth, m_patchHeight);
    }

    setMaximumWidth(QWIDGETSIZE_MAX);
    setMinimumWidth(1);
    setMaximumHeight(QWIDGETSIZE_MAX);
    setMinimumHeight(1);

    if(m_allowScrolling && m_direction == Horizontal) {
        setMaximumHeight(m_numRows*m_patchHeight);
        setMinimumHeight(m_numRows*m_patchHeight);
    }

    if(m_allowScrolling && m_direction == Vertical) {
        setMaximumWidth(m_numCols*m_patchWidth);
        setMinimumWidth(m_numCols*m_patchWidth);
    }

    if(m_allowScrolling == false) {
        m_scrollValue = 0;
    }


    QResizeEvent dummy(size(), QSize(-1,-1));
    resizeEvent(&dummy);

    setPopupBehaviour(false, false);
    update();
}

int KisColorPatches::widthOfAllPatches()
{
    return (fieldCount()/m_numRows)*m_patchWidth;
}

int KisColorPatches::heightOfAllPatches()
{
    return (fieldCount()/m_numCols)*m_patchHeight;
}

int KisColorPatches::heightForWidth(int width) const
{
    int numPatchesInARow = width / m_patchWidth;
    int numRows = qMax((fieldCount() - 1), 1) / qMax(numPatchesInARow + 1, 1);
    return qMax(numRows * m_patchHeight, m_patchHeight);
}

int KisColorPatches::widthForHeight(int height) const
{
    if (height == 0) {
        return 0;
    }

    if (m_patchHeight == 0) {
        return 0;
    }
    int numPatchesInACol = height / m_patchHeight;

    int numCols = (fieldCount() - 1) / (numPatchesInACol + 1);

    return qMax(numCols*m_patchWidth, m_patchWidth);
}

int KisColorPatches::fieldCount() const
{
    return m_patchCount+m_buttonList.size();
}

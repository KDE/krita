/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shicmap@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KIS_POPUP_PALETTE_H
#define KIS_POPUP_PALETTE_H

#include <kis_types.h>
#include <QWidget>
#include <QQueue>
#include <KoColorDisplayRendererInterface.h>


class KisFavoriteBrushData;
class KisFavoriteResourceManager;
class QWidget;
class KoColor;
class KoTriangleColorSelector;
class KisSignalCompressor;

class KisPopupPalette : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(int hoveredPreset READ hoveredPreset WRITE setHoveredPreset)
    Q_PROPERTY(int hoveredColor READ hoveredColor WRITE setHoveredColor)
    Q_PROPERTY(int selectedColor READ selectedColor WRITE setSelectedColor)

public:
    KisPopupPalette(KisFavoriteResourceManager*, const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(), QWidget *parent = 0);
    ~KisPopupPalette();
    QSize sizeHint() const;

    void showPopupPalette(const QPoint&);
    void showPopupPalette(bool b);

    //functions to set up selectedBrush
    void setSelectedBrush(int x);
    int selectedBrush() const;
    //functions to set up selectedColor
    void setSelectedColor(int x);
    int selectedColor() const;

protected:

    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);

    //functions to calculate index of favorite brush or recent color in array
    //n is the total number of favorite brushes or recent colors
    int calculateIndex(QPointF, int n);

    int calculatePresetIndex(QPointF, int n);

    //functions to set up hoveredBrush
    void setHoveredPreset(int x);
    int hoveredPreset() const;
    //functions to set up hoveredColor
    void setHoveredColor(int x);
    int hoveredColor() const;


private:
    void setVisible(bool b);

    QPainterPath drawDonutPathFull(int, int, int, int);
    QPainterPath drawDonutPathAngle(int, int, int);
    bool isPointInPixmap(QPointF&, int pos);

    QPainterPath pathFromPresetIndex(int index);

    int numSlots();
private:

    int m_hoveredPreset;
    int m_hoveredColor;
    int m_selectedColor;
    KisFavoriteResourceManager* m_resourceManager;
    KoTriangleColorSelector* m_triangleColorSelector;

    QTimer* m_timer;

    const KoColorDisplayRendererInterface *m_displayRenderer;

    KisSignalCompressor *m_colorChangeCompressor;

signals:
    void sigChangeActivePaintop(int);
    void sigUpdateRecentColor(int);
    void sigChangefGColor(const KoColor&);

    // These are used to handle a bug:
    // If pop up palette is visible and a new colour is selected, the new colour
    // will be added when the user clicks on the canvas to hide the palette
    // In general, we want to be able to store recent color if the pop up palette
    // is not visible
    void sigEnableChangeFGColor(bool);
    void sigTriggerTimer();

private slots:
    void slotExternalFgColorChanged(const KoColor &color);
    void slotEmitColorChanged();
    void slotSetSelectedColor(int x) { setSelectedColor(x); update(); }
    void slotTriggerTimer();
    void slotEnableChangeFGColor();
    void slotUpdate() { update(); }
    void slotHide() { showPopupPalette(false); }
};

#endif // KIS_POPUP_PALETTE_H

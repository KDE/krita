/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_COLSELNG_COLOR_PATCHES_H
#define KIS_COLSELNG_COLOR_PATCHES_H

#include <QWidget>

class KisColorPatches : public QWidget
{
Q_OBJECT
public:
    explicit KisColorPatches(QWidget *parent = 0, QString configPrefix=QString("lastUsedColors"));
    enum Direction { Horizontal, Vertical };

    void setColors(QList<QColor> colors);
signals:

public slots:
    void updateSettings();
protected:
    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent *);
    void resizeEvent(QResizeEvent *);
public:
    /// set buttons, that should be drawn additionally to the patches
    /// this class takes ownership of them and will delete them
    /// they will be resized to the patchsize
    void setAdditionalButtons(QList<QWidget*> buttonList);
private:
    int m_patchWidth;
    int m_patchHeight;
    int m_numPatches;
    QList<QColor> m_colors;
    int m_scrollValue;

    Direction m_direction;
    bool m_allowScrolling;
    int m_numCols;
    int m_numRows;
    QList<QWidget*> m_buttonList;

    /// returns width of the patchfield, if there are only m_numRows allowed
    int widthOfAllPatches();
    /// returns height of the patchfield, if there are only m_numCols allowed
    int heightOfAllPatches();

    /// returns height, that is needed to display all patches with the given width
    int heightForWidth(int width) const;
    /// returns width, that is needed to display all patches with the given height
    int widthForHeight(int height) const;
    
    /// returns count of colors and buttons
    int fieldCount() const;

    QString m_configPrefix;
};

#endif // KIS_COLSELNG_COLOR_PATCHES_H

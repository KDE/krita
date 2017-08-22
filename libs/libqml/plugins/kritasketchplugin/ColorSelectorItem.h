/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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


#ifndef COLORSELECTORITEM_H
#define COLORSELECTORITEM_H

#include <QQuickPaintedItem>

class KoColor;
class ColorSelectorItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged)
    Q_PROPERTY(bool changeBackground READ changeBackground WRITE setChangeBackground NOTIFY changeBackgroundChanged)
    // This is a heuristic assistance property - if this is set to false, the item will not be painted
    Q_PROPERTY(bool shown READ shown WRITE setShown NOTIFY shownChanged)
public:
    explicit ColorSelectorItem(QQuickItem* parent = 0);
    virtual ~ColorSelectorItem();
    virtual void paint(QPainter* painter);

    QObject* view() const;
    void setView(QObject* newView);

    bool changeBackground() const;
    void setChangeBackground(bool newChangeBackground);

    bool shown() const;
    void setShown(bool newShown);

    Q_INVOKABLE void setAlpha(int percentValue);

Q_SIGNALS:
    void viewChanged();
    void changeBackgroundChanged();
    void shownChanged();
    void colorChanged(QColor newColor, qreal newAlpha, bool backgroundChanged);

protected:
    virtual void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseEvent(QMouseEvent* event);

private Q_SLOTS:
    void fgColorChanged(const KoColor& newColor);
    void bgColorChanged(const KoColor& newColor);
    void repaint();

private:
    class Private;
    Private* d;
};

#endif // COLORSELECTORITEM_H

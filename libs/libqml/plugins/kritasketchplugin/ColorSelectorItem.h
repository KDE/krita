/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit ColorSelectorItem(QQuickItem *parent = nullptr);
    ~ColorSelectorItem() override;
    void paint(QPainter *painter) override;

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
    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
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

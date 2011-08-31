/* This file is part of the KDE project
*
* Copyright (C) 2008 Peter Penz <peter.penz19@gmail.com>
* Copyright (C) 2011 Paul Mendez <paulestebanms@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#ifndef KOSELECTIONTOGGLE_H
#define KOSELECTIONTOGGLE_H

#include <QAbstractButton>
#include <QPixmap>
#include <QModelIndex>
#include "kowidgets_export.h"

class QTimeLine;

/**
 * @brief Toggle button for changing the selection of an hovered item.
 *
 * The toggle button is visually invisible until it is displayed at least
 * for one second.
 *
 * @see SelectionManager
 */
class KOWIDGETS_EXPORT KoSelectionToggle : public QAbstractButton
{
    Q_OBJECT

public:
    explicit KoSelectionToggle(QWidget *parent);
    virtual ~KoSelectionToggle();
    virtual QSize sizeHint() const;

    /**
     * Resets the selection toggle so that it is hidden and stays
     * visually invisible for at least one second after it is shown again.
     */
    void reset();

    void setIndex(const QModelIndex &index);
    QModelIndex index() const;

    /**
     * Sets the margin around the selection-icon in pixels. Per default
     * the value is 0.
     */
    void setMargin(int margin);
    int margin() const;

public slots:
    virtual void setVisible(bool visible);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private slots:
    /**
     * Sets the alpha value for the fading animation and is
     * connected with m_fadingTimeLine.
     */
    void setFadingValue(int value);

    void setIconOverlay(bool checked);
    void refreshIcon();

private:
    void startFading();
    void stopFading();

private:
    bool m_isHovered;
    bool m_leftMouseButtonPressed;
    int m_fadingValue;
    int m_margin;
    QPixmap m_icon;
    QTimeLine *m_fadingTimeLine;
    QModelIndex m_index;
};

#endif // KOSELECTIONTOGGLE_H

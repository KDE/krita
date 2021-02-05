/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SCRATCH_PAD_EVENT_FILTER_H
#define __KIS_SCRATCH_PAD_EVENT_FILTER_H

#include <QObject>
#include <QTransform>
#include <KoPointerEvent.h>

class QWidget;
class KisScratchPad;


class KisScratchPadEventFilter : public QObject
{
    Q_OBJECT

public:
    KisScratchPadEventFilter(QWidget *parent);
    void setWidgetToDocumentTransform(const QTransform &transform);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    inline QWidget* parentWidget();
    inline KoPointerEvent* createMouseEvent(QEvent *event);
    inline KoPointerEvent* createTabletEvent(QEvent *event);

private:
    QTransform m_widgetToDocument;
    bool m_tabletPressed;

    Qt::MouseButton m_pressedButton;
    KisScratchPad *m_scratchPad;
};

#endif /* __KIS_SCRATCH_PAD_EVENT_FILTER_H */

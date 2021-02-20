/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

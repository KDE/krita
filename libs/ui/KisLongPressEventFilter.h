// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef __KISLONGPRESSEVENTFILTER_H
#define __KISLONGPRESSEVENTFILTER_H
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QWidget>

class QMouseEvent;
class QTimer;

class KisLongPressEventFilter : public QObject
{
    Q_OBJECT
public:
    static constexpr char ENABLED_PROPERTY[] = "KRITA_LONG_PRESS";

    explicit KisLongPressEventFilter(QObject *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    static constexpr int MINIMUM_DELAY = 100;
    static constexpr int MINIMUM_DISTANCE = 0;

    void handleMousePress(QWidget *target, const QMouseEvent *me);
    void handleMouseMove(const QMouseEvent *me);
    void cancel();

    bool isWithinDistance(const QPoint &globalPos) const;

    void triggerLongPress();

    static bool isContextMenuTarget(QWidget *target);
    static bool isLongPressableWidget(QWidget *target);

    QTimer *m_timer;
    long long m_distanceSquared = 0LL;
    QPoint m_pressLocalPos;
    QPoint m_pressGlobalPos;
    QPointer<QWidget> m_target;
};

#endif

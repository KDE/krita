#ifndef KISMOUSECLICKEATER_H
#define KISMOUSECLICKEATER_H

#include <QtGlobal>
#include <QObject>
#include <QElapsedTimer>


class KisMouseClickEater : public QObject
{
public:
    KisMouseClickEater(Qt::MouseButtons buttons,
                       int clicksToEat = 1,
                       QObject *parent = 0);

    ~KisMouseClickEater();

    void reset();
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Qt::MouseButtons m_buttons = Qt::NoButton;
    int m_clicksToEat = 1;
    int m_clicksHappened = 0;
    bool m_blockTimedRelease = false;
    QElapsedTimer m_timeSinceReset;
};

#endif // KISMOUSECLICKEATER_H

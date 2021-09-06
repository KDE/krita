/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Christian Muehlhaeuser <chris@chris.de>
 *  SPDX-FileCopyrightText: 2004-2006 Seb Ruiz <ruiz@kde.org>
 *  SPDX-FileCopyrightText: 2004, 2005 Max Howell <max.howell@methylblue.com>
 *  SPDX-FileCopyrightText: 2005 Gabor Lehel <illissius@gmail.com>
 *  SPDX-FileCopyrightText: 2008, 2009 Mark Kretschmann <kretschmann@kde.org>
 *  SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 */
#include "kis_floating_message.h"

#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QRegExp>
#include <QDesktopWidget>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

#include <kis_icon.h>
#include <kis_debug.h>
#include "kis_global.h"


#define OSD_WINDOW_OPACITY 0.85

static void addDropShadow(QWidget *widget, QColor color)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(widget);
    effect->setBlurRadius(6);
    effect->setOffset(0);
    effect->setColor(color);
    widget->setGraphicsEffect(effect);
}

static Qt::AlignmentFlag flagsToAlignmentFlags(int flags)
{
    constexpr int mask = Qt::AlignLeft
                       | Qt::AlignRight
                       | Qt::AlignHCenter
                       | Qt::AlignJustify
                       | Qt::AlignTop
                       | Qt::AlignBottom
                       | Qt::AlignVCenter
                       | Qt::AlignCenter;
    return Qt::AlignmentFlag(flags & mask);
}

KisFloatingMessage::KisFloatingMessage(const QString &message, QWidget *parent, bool showOverParent, int timeout, Priority priority, int alignment)
    : QWidget(parent)
    , m_message(message)
    , m_showOverParent(showOverParent)
    , m_timeout(timeout)
    , m_priority(priority)
    , m_alignment(alignment)
    , widgetQueuedForDeletion(false)
{
    m_icon = KisIconUtils::loadIcon("krita-branding").pixmap(256, 256).toImage();

    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowTransparentForInput);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating);

    m_messageLabel = new QLabel(message, this);
    m_messageLabel->setAttribute(Qt::WA_TranslucentBackground);
    m_iconLabel = new QLabel(this);
    m_iconLabel->setAttribute(Qt::WA_TranslucentBackground);
    {
        int h, s, v;
        palette().color( QPalette::Normal, QPalette::Foreground ).getHsv( &h, &s, &v );
        const QColor shadowColor = v > 128 ? Qt::black : Qt::white;
        addDropShadow(m_messageLabel, shadowColor);
        addDropShadow(m_iconLabel, shadowColor);
    }

    m_timer.setSingleShot( true );
    connect(&m_timer, SIGNAL(timeout()), SLOT(startFade()));
    connect(this, SIGNAL(destroyed()), SLOT(widgetDeleted()));
}

void KisFloatingMessage::tryOverrideMessage(const QString message,
                                            const QIcon& icon,
                                            int timeout,
                                            KisFloatingMessage::Priority priority,
                                            int alignment)
{
    if ((int)priority > (int)m_priority) return;

    m_message = message;
    m_messageLabel->setText(message);
    setIcon(icon);
    m_timeout = timeout;
    m_priority = priority;
    m_alignment = alignment;
    showMessage();
    update();
}

void KisFloatingMessage::showMessage()
{
    if (widgetQueuedForDeletion) return;

    m_messageLabel->setAlignment(flagsToAlignmentFlags(m_alignment));
    m_messageLabel->setWordWrap(m_alignment & Qt::TextWordWrap);
    m_messageLabel->adjustSize();

    QRect geom;
#if QT_VERSION >= QT_VERSION_CHECK(5,13,0)
    geom = determineMetrics(fontMetrics().horizontalAdvance('x'));
#else
    geom = determineMetrics(fontMetrics().width('x'));
#endif
    setGeometry(geom);
    setWindowOpacity(OSD_WINDOW_OPACITY);

    QRect rect(QPoint(), geom.size());
    rect.adjust(m_m, m_m, -m_m, -m_m);
    if (!m_icon.isNull()) {
        QRect r(rect);
        r.setTop((size().height() - m_scaledIcon.height() ) / 2);
        r.setSize(m_scaledIcon.size());
        m_iconLabel->setPixmap(m_scaledIcon);
        m_iconLabel->setFixedSize(r.size());
        m_iconLabel->move(r.topLeft());
        m_iconLabel->show();
        rect.setLeft(rect.left() + m_scaledIcon.width() + m_m);
    } else {
        m_iconLabel->hide();
    }
    m_messageLabel->setFixedSize(rect.size());
    m_messageLabel->move(rect.topLeft());

    QWidget::setVisible(true);
    m_fadeTimeLine.stop();
    m_timer.start(m_timeout);
}

void KisFloatingMessage::setShowOverParent(bool show)
{
    m_showOverParent = show;
}

void KisFloatingMessage::setIcon(const QIcon& icon)
{
    m_icon = icon.pixmap(256, 256).toImage();
}

const int MARGIN = 20;

QRect KisFloatingMessage::determineMetrics( const int M )
{
    m_m = M;

    const QSize minImageSize = m_icon.size().boundedTo(QSize(100, 100));

    // determine a sensible maximum size, don't cover the whole desktop or cross the screen
    const QSize margin( (M + MARGIN) * 2, (M + MARGIN) * 2); //margins
    const QSize image = m_icon.isNull() ? QSize(0, 0) : minImageSize;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect geom = parentWidget()->geometry();
    QPoint p(geom.width() / 2 + geom.left(), geom.height() / 2 + geom.top());
    QScreen *s = qApp->screenAt(p);
    QSize max;
    if (s) {
        max = QSize(s->availableGeometry().size() - margin);
    }
    else {
        max = QSize(1024, 768);
    }
#else
    const QSize max = QApplication::desktop()->availableGeometry(parentWidget()).size() - margin;
#endif


    // If we don't do that, the boundingRect() might not be suitable for drawText() (Qt issue N67674)
    m_message.replace(QRegExp( " +\n"), "\n");
    // remove consecutive line breaks
    m_message.replace(QRegExp( "\n+"), "\n");

    // The osd cannot be larger than the screen
    QRect rect = fontMetrics().boundingRect(0, 0, max.width() - image.width(), max.height(),
           m_alignment, m_message);

    if (!m_icon.isNull()) {
        const int availableWidth = max.width() - rect.width() - M; //WILL be >= (minImageSize.width() - M)

        m_scaledIcon = QPixmap::fromImage(m_icon.scaled(qMin(availableWidth, m_icon.width()),
                                                        qMin( rect.height(), m_icon.height()),
                                                        Qt::KeepAspectRatio, Qt::SmoothTransformation));

        const int widthIncludingImage = rect.width() + m_scaledIcon.width() + M; //margin between text + image
        rect.setWidth( widthIncludingImage );
    }

    // expand in all directions by 2*M
    //
    // take care with this rect, because it must be *bigger*
    // than the rect we paint the message in
    rect = kisGrowRect(rect, 2 * M);



    const QSize newSize = rect.size();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect screen;
    if (s) {
        screen = s->availableGeometry();
    }
    else {
        screen = QRect(0, 0, 1024, 768);
    }
#else
    QRect screen = QApplication::desktop()->screenGeometry(parentWidget());
#endif


    QPoint newPos(MARGIN, MARGIN);

    if (parentWidget() && m_showOverParent) {
        screen = parentWidget()->geometry();
        screen.setTopLeft(parentWidget()->mapToGlobal(QPoint(MARGIN, MARGIN + 50)));
        newPos = screen.topLeft();
    }
    else {
        // move to the right
        newPos.rx() = screen.width() - MARGIN - newSize.width();

        //ensure we don't dip below the screen
        if (newPos.y() + newSize.height() > screen.height() - MARGIN) {
            newPos.ry() = screen.height() - MARGIN - newSize.height();
        }
        // correct for screen position
        newPos += screen.topLeft();

        if (parentWidget()) {
            // Move a bit to the left as there could be a scrollbar
            newPos.setX(newPos.x() - MARGIN);
        }
    }

    QRect rc(newPos, rect.size());

    return rc;
}

void KisFloatingMessage::startFade()
{
    m_fadeTimeLine.setDuration(500);
    m_fadeTimeLine.setEasingCurve(QEasingCurve::InCurve);
    m_fadeTimeLine.setLoopCount(1);
    m_fadeTimeLine.setFrameRange(0, 10);
    connect(&m_fadeTimeLine, SIGNAL(finished()), SLOT(removeMessage()));
    connect(&m_fadeTimeLine, SIGNAL(frameChanged(int)), SLOT(updateOpacity(int)));
    m_fadeTimeLine.start();
}

void KisFloatingMessage::removeMessage()
{
    m_timer.stop();
    m_fadeTimeLine.stop();
    widgetQueuedForDeletion = true;

    hide();
    deleteLater();
}

void KisFloatingMessage::updateOpacity(int value)
{
    setWindowOpacity(OSD_WINDOW_OPACITY / 10.0 * (10 - value));
}

void KisFloatingMessage::widgetDeleted()
{
    widgetQueuedForDeletion = false;
}

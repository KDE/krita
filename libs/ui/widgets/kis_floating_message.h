/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Christian Muehlhaeuser <chris@chris.de>
 *  SPDX-FileCopyrightText: 2004-2006 Seb Ruiz <ruiz@kde.org>
 *  SPDX-FileCopyrightText: 2004, 2005 Max Howell <max.howell@methylblue.com>
 *  SPDX-FileCopyrightText: 2005 Gabor Lehel <illissius@gmail.com>
 *  SPDX-FileCopyrightText: 2008, 2009 Mark Kretschmann <kretschmann@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_FLOATING_MESSAGE_H
#define KIS_FLOATING_MESSAGE_H

#include <QWidget>
#include <QString>
#include <QImage>
#include <QTimer>
#include <QTimeLine>

#include <kritaui_export.h>

/**
 * @brief The KisFloatingMessage class shows the given message in a semi-transparent
 * bubble that doesn't take focus and slowly fades away.
 *
 * Heavily based on Amarok's Osd.cpp
 */
class KRITAUI_EXPORT KisFloatingMessage : public QWidget
{
    Q_OBJECT

public:
    enum Priority {
        High = 0,
        Medium,
        Low
    };

    explicit KisFloatingMessage(const QString &message, QWidget *parent, bool showOverParent, int timeout,
                                Priority priority, int alignment = Qt::AlignCenter | Qt::TextWordWrap);

    /// Show message above parent widget instead of screen
    void setShowOverParent(bool show);

    void setIcon(const QIcon& icon);

    void tryOverrideMessage(const QString message,
                            const QIcon& icon,
                            int timeout,
                            KisFloatingMessage::Priority priority,
                            int alignment = Qt::AlignCenter | Qt::TextWordWrap);

protected:

    void paintEvent(QPaintEvent *e) override;

public Q_SLOTS:
    void showMessage();
    void removeMessage();

private Q_SLOTS:
    void startFade();
    void updateOpacity(int value);
    void widgetDeleted();
private:

    QRect determineMetrics(const int M);

    QString m_message;
    QImage m_icon;
    QPixmap m_scaledIcon;
    QTimer m_timer;
    int m_m;
    QTimeLine m_fadeTimeLine;
    bool m_showOverParent;
    int m_timeout;
    Priority m_priority;
    int m_alignment;
    bool widgetQueuedForDeletion;
};

#endif // KIS_FLOATING_MESSAGE_H

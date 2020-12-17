/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BUSY_PROGRESS_INDICATOR_H
#define __KIS_BUSY_PROGRESS_INDICATOR_H

#include <QObject>
#include <QScopedPointer>

class KoProgressProxy;

class KisBusyProgressIndicator : public QObject
{
    Q_OBJECT
public:
    explicit KisBusyProgressIndicator(KoProgressProxy *progressProxy);
    ~KisBusyProgressIndicator() override;

public:
    /**
     * To be called when progressProxy is and will be no longer available
     * and this object is going to be deleted as well.
     */
    void prepareDestroying();

public Q_SLOTS:
    /**
     * Trigger update of progress state.
     */
    void update();

private Q_SLOTS:
    /**
     * Call only via emitting sigStartTimer, to ensure it is called in
     * the context of the QObject's thread.
     */
    void slotStartTimer();
    void timerFinished();

Q_SIGNALS:
    void sigStartTimer();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_BUSY_PROGRESS_INDICATOR_H */

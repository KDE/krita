/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KO_UPDATERPRIVATE__P_H
#define KO_UPDATERPRIVATE__P_H

#include "KoProgressUpdater.h"

#include <QTime>
#include <QVector>

/**
 * KoUpdaterPrivate is the gui-thread side of KoUpdater. Communication
 * between KoUpdater and KoUpdaterPrivate is handled through queued
 * connections -- this is the main app thread part of the
 * KoUpdater-KoUpdaterPrivate bridge.
 *
 * The gui thread can iterate over its list of KoUpdaterPrivate
 * instances for the total progress computation: the queued signals
 * from the threads will only arrive when the eventloop in the gui
 * thread has a chance to deliver them.
 */
class KoUpdaterPrivate : public QObject
{

    Q_OBJECT

public:

    KoUpdaterPrivate(KoProgressUpdater *parent, int weight, const QString& name, bool isPersistent = false);

    /// when deleting an updater, make sure the accompanying thread is
    /// interrupted, too.
    ~KoUpdaterPrivate() override;

    bool interrupted() const { return m_interrupted; }

    int progress() const { return m_progress; }

    int weight() const { return m_weight; }

    QString autoNestedName() const;
    QString subTaskName() const;
    QString mergedSubTaskName() const;

    bool hasValidRange() const;
    bool isPersistent() const;
    bool isCompleted() const;

    QPointer<KoUpdater> connectedUpdater() const;

public Q_SLOTS:

    /// Cancel comes from KoUpdater
    void cancel();

    void setInterrupted(bool value = true);

    /// progress comes from KoUpdater
    void setProgress( int percent );

    void setAutoNestedName(const QString &name);
    void setHasValidRange(bool value);


Q_SIGNALS:

    /// Emitted whenever the progress changed
    void sigUpdated();

    /// Emitted whenever the parent KoProgressUpdater is interrupted,
    /// for instance through a press on a cancel button
    void sigInterrupted(bool value);

private:
    int m_progress; // always in percent
    int m_weight;
    bool m_interrupted;
    QString m_autoNestedName;
    QString m_subTaskName;
    bool m_hasValidRange;
    bool m_isPersistent;
    KoProgressUpdater *m_parent;
    QPointer<KoUpdater> m_connectedUpdater;
};

#endif

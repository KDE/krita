/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISWAYLANDDEBUGINFOFETCHER_H
#define KISWAYLANDDEBUGINFOFETCHER_H

#include <QObject>

class KisWaylandAPIColorManager;

class KisWaylandDebugInfoFetcher : public QObject
{
    Q_OBJECT
public:
    KisWaylandDebugInfoFetcher(QObject *parent = nullptr);
    ~KisWaylandDebugInfoFetcher();

    bool isReady() const;
    QString report() const;

private:
    void reinitialize();
    QString generateReport();

Q_SIGNALS:
    void sigDebugInfoReady(const QString &report);

private:
    std::shared_ptr<KisWaylandAPIColorManager> m_waylandManager;
    bool m_isReady {false};
    QString m_report;
};

#endif /* KISWAYLANDDEBUGINFOFETCHER_H */
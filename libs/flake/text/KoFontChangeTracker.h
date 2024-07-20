/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFONTCHANGETRACKER_H
#define KOFONTCHANGETRACKER_H

#include <QObject>

/**
 * @brief The KoFontChangeTracker class
 * This class keeps track of the FontConfig refresh value,
 * as well as the paths FontConfig is looking at,
 * and resets the
 */
class KoFontChangeTracker : public QObject
{
    Q_OBJECT
public:
    explicit KoFontChangeTracker(QStringList paths, int configRefreshValue = 0, QObject *parent = nullptr);
    ~KoFontChangeTracker();

    /// This should be called after fontregistery initialization is done to start the signal compressor.
    void resetChangeTracker();

    /// This needs to be manually connected.
    void connectToRegistery();
public Q_SLOTS:
    void updateFontRegistery();
Q_SIGNALS:
    void sigUpdateConfig();
private Q_SLOTS:
    void intervalElapsed();
    void directoriesChanged(QString path);
    void testUpdate();
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOFONTCHANGETRACKER_H

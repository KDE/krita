/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPROGRESSBAR_H
#define KOPROGRESSBAR_H

#include <QProgressBar>
#include <KoProgressProxy.h>
#include "kritawidgetutils_export.h"

/**
 * KoProgressBar is a thin wrapper around QProgressBar that also implements
 * the abstract base class KoProgressProxy. Use this class, not QProgressBar
 * to pass to KoProgressUpdater.
 */
class KRITAWIDGETUTILS_EXPORT KoProgressBar : public QProgressBar, public KoProgressProxy
{
    Q_OBJECT
public:

    explicit KoProgressBar(QWidget *parent = 0);

    ~KoProgressBar() override;

    int maximum() const override;
    void setValue(int value) override;
    void setRange(int minimum, int maximum) override;
    void setFormat(const QString &format) override;

Q_SIGNALS:

    void done();
};

#endif

/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef LEVEL_H
#define LEVEL_H

#include <QObject>
#include <QVariant>


class LevelFilter : public QObject
{
    Q_OBJECT
public:
    LevelFilter(QObject *parent, const QVariantList &);
    ~LevelFilter() override;
};

#endif

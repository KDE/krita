/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _FILTEROP_H_
#define _FILTEROP_H_

#include <QObject>
#include <QVariant>

class FilterOp : public QObject
{
    Q_OBJECT
public:
    FilterOp(QObject *parent, const QVariantList &);
    ~FilterOp() override;
};

#endif

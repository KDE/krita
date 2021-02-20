/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CLONESARRAY_H
#define CLONESARRAY_H

#include <KisActionPlugin.h>
#include <kis_types.h>

class ClonesArray : public KisActionPlugin
{
    Q_OBJECT
public:
    ClonesArray(QObject *parent, const QVariantList &);
    ~ClonesArray() override;

private Q_SLOTS:
    void slotCreateClonesArray();
};

#endif // CLONESARRAY<_H

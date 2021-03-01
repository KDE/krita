/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINTERSTROKEDATA_H
#define KISINTERSTROKEDATA_H

#include <kritaimage_export.h>
#include <QSharedPointer>

class KUndo2Command;


class KRITAIMAGE_EXPORT KisInterstrokeData
{
public:
    virtual ~KisInterstrokeData();

    virtual void beginTransaction() = 0;
    virtual KUndo2Command* endTransaction() = 0;
};

using KisInterstrokeDataSP = QSharedPointer<KisInterstrokeData>;


#endif // KISINTERSTROKEDATA_H

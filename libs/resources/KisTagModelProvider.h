/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TAG_MODEL_PROVIDER_H
#define KIS_TAG_MODEL_PROVIDER_H

#include <QObject>
#include <QAbstractTableModel>

#include <KisTag.h>
#include <KoResource.h>

#include "kritaresources_export.h"
#include "KisTagModel.h"
#include "KisTagResourceModel.h"

class KRITARESOURCES_EXPORT KisTagModelProvider : public QObject
{
    Q_OBJECT

public:

    KisTagModelProvider();
    ~KisTagModelProvider();

    static KisTagModel *tagModel(const QString& resourceType);
    static KisTagResourceModel *tagResourceModel(const QString& resourceType);

private:

    struct Private;
    Private* const d;

};

#endif // KIS_TAG_MODEL_PROVIDER_H

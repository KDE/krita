/*
 * Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_TAG_MODEL_PROVIDER_H
#define KIS_TAG_MODEL_PROVIDER_H


#include <QObject>
#include <QAbstractTableModel>

#include <KisTag.h>
#include <KoResource.h>

#include "kritaresources_export.h"
#include "KisTagModel.h"



class KRITARESOURCES_EXPORT KisTagModelProvider : public QObject
{
    Q_OBJECT

public:

    KisTagModelProvider();
    ~KisTagModelProvider();

    static KisTagModel* tagModel(const QString& resourceType);
    static void resetModels();
    static void resetModel(const QString& resourceType);

private:

    struct Private;
    Private* const d;

};

#endif // KIS_TAG_MODEL_PROVIDER_H

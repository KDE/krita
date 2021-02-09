/*
 *  SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_RESOURCE_SEARCH_BOX_FILTER_H
#define KIS_RESOURCE_SEARCH_BOX_FILTER_H


#include "kritaresources_export.h"

#include <QString>
#include <QScopedPointer>

/**
 * XXX: Apidox
 * 
 */
class KRITARESOURCES_EXPORT KisResourceSearchBoxFilter
{

public:

    KisResourceSearchBoxFilter();
    ~KisResourceSearchBoxFilter();
    void setFilter(const QString& filter);
    bool matchesResource(const QString& resourceName, const QStringList &tagList);
    bool isEmpty();

private:

    void initializeFilterData();
    void clearFilterData();

    class Private;
    QScopedPointer<Private> d;

};


#endif

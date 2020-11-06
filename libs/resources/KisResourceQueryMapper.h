#ifndef KISRESOURCEQUERYMAPPER_H
#define KISRESOURCEQUERYMAPPER_H

#include <QSqlQuery>

class KisResourceQueryMapper
{
public:
    /**
     * @brief variantFromResourceQuery returns a QVariant for the given column and or role
     * @param query the query: it supposed to be in the right position already
     */
    static QVariant variantFromResourceQuery(const QSqlQuery &query, int column, int role);

    static QVariant variantFromResourceQueryById(int resourceId, int column, int role);
};

#endif // KISRESOURCEQUERYMAPPER_H

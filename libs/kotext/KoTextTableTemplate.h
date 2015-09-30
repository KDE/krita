/* This file is part of the KDE project
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOTEXTTABLETEMPLATE_H
#define KOTEXTTABLETEMPLATE_H

#include "kritatext_export.h"

#include <QObject>

#include <KoXmlReaderForward.h>

class KoXmlWriter;
class KoShapeLoadingContext;
class KoTextSharedSavingData;

class KRITATEXT_EXPORT KoTextTableTemplate : public QObject
{
public:
    enum Property {
        StyleId = 0,
        BackGround,
        Body,
        EvenColumns,
        EvenRows,
        FirstColumn,
        FirstRow,
        LastColumn,
        LastRow,
        OddColumns,
        OddRows
    };

    int background() const;
    void setBackground(int styleId);

    int body() const;
    void setBody(int styleId);

    int evenColumns() const;
    void setEvenColumns(int styleId);

    int evenRows() const;
    void setEvenRows(int styleId);

    int firstColumn() const;
    void setFirstColumn(int styleId);

    int firstRow() const;
    void setFirstRow(int styleId);

    int lastColumn() const;
    void setLastColumn(int styleId);

    int lastRow() const;
    void setLastRow(int styleId);

    int oddColumns() const;
    void setOddColumns(int styleId);

    int oddRows() const;
    void setOddRows(int styleId);

    /// Constructor
    explicit KoTextTableTemplate(QObject *parent = 0);

    /// Destructor
    ~KoTextTableTemplate();

    /// return the name of the style.
    QString name() const;

    /// set a user-visible name on the style.
    void setName(const QString &name);

    /// each style has a unique ID (non persistent) given out by the styleManager
    int styleId() const;

    /// each style has a unique ID (non persistent) given out by the styleManager
    void setStyleId(int id);

    /**
     * Load the template style from the element
     *
     * @param context the odf loading context
     * @param element the element containing the
     */
    void loadOdf(const KoXmlElement *element, KoShapeLoadingContext &context);

    /**
     * save the table-template element
     */
    void saveOdf(KoXmlWriter *writer, KoTextSharedSavingData *savingData) const;


private:
    class Private;
    Private * const d;

};


#endif //KOTEXTTABLETEMPLATE_H

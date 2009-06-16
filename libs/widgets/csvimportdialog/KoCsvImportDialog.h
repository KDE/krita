/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 2004 Nicolas GOUTTE <goutte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KO_CSV_IMPORT_DIALOG
#define KO_CSV_IMPORT_DIALOG

#include <kdialog.h>

#include "kowidgets_export.h"

/**
 * A dialog to choose the options for importing CSV data.
 */
class KOWIDGETS_EXPORT KoCsvImportDialog : public KDialog
{
    Q_OBJECT
public:
    /**
     * The type the data should be treated as.
     */
    enum DataType
    {
        Generic     = 0x01, ///< Parses the content; it may become text, number, date, ...
        Text        = 0x02, ///< Forces the content to a text value.
        Date        = 0x04, ///< Tries to convert the content to a date value.
        Currency    = 0x08, ///< Tries to convert the content to a currency value.
        None        = 0x10  ///< Skips the content.
    };
    Q_DECLARE_FLAGS(DataTypes, DataType)

    /**
     * Constructor.
     */
    KoCsvImportDialog(QWidget* parent);

    /**
     * Destructor.
     */
    virtual ~KoCsvImportDialog();

    /**
     * Set the data to import.
     */
    void setData(const QByteArray& data);

    /**
     * \return whether the first row is a header row
     */
    bool firstRowContainHeaders() const;

    /**
     * \return whether the first column is a header column
     */
    bool firstColContainHeaders() const;

    /**
     * \return the number of rows to import
     */
    int rows() const;

    /**
     * \return the number of columns to import
     */
    int cols() const;

    /**
     * The data type of column \p col.
     */
    DataType dataType(int col) const;

    /**
     * The text at \p row, \p col.
     */
    QString  text(int row, int col) const;

    /**
     * Sets the data types, that should be selectable.
     */
    void setDataTypes(DataTypes dataTypes);

    /**
     * Enables or disables the data widget.
     */
    void setDataWidgetEnabled(bool enable);

    /**
     * \return the decimal symbol
     */
    QString decimalSymbol() const;

    /**
     * Sets the decimal symbol.
     */
    void setDecimalSymbol(const QString& symbol);

    /**
     * \return the thousands separator
     */
    QString thousandsSeparator() const;

    /**
     * Sets the thousands separator.
     */
    void setThousandsSeparator(const QString& separator);

protected slots:
    void returnPressed();
    void formatChanged(const QString&);
    void delimiterClicked(int id);
    void textquoteSelected(const QString& mark);
    void currentCellChanged(int, int col);
    void genericDelimiterChanged(const QString &);
    void ignoreDuplicatesChanged(int);
    void updateClicked();
    void encodingChanged(const QString &);

private:
    Q_DISABLE_COPY(KoCsvImportDialog)

    class Private;
    Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoCsvImportDialog::DataTypes)

#endif // KO_CSV_IMPORT_DIALOG

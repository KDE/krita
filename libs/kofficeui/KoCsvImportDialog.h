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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KO_CSV_IMPORT_DIALOG
#define KO_CSV_IMPORT_DIALOG

#include <QStringList>
#include <QWidget>

#include <kdialog.h>

#include "koffice_export.h"

#include "ui_KoCsvImportDialog.h"

class KoCsvImportWidget : public QWidget, public Ui::KoCsvImportWidget
{
  public:
    KoCsvImportWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }
};

class KOFFICEUI_EXPORT KoCsvImportDialog : public KDialog
{
    Q_OBJECT
public:
    enum Header
    {
        TEXT,       ///< Normal text
        NUMBER,     ///< Number (either like locale or like C)
        DATE,       ///< Date \todo What type exactly?
        CURRENCY,   ///< Currency
        COMMANUMBER,///< Number, which decimal symbol is a comma
        POINTNUMBER ///< Number, which decimal symbol is a point/dot
    };

    KoCsvImportDialog(QWidget* parent);
    ~KoCsvImportDialog();

    void     setData( const QByteArray& data);
    bool     firstRowContainHeaders();
    bool     firstColContainHeaders();
    int      rows();
    int      cols();
    int      headerType(int col);
    QString  text(int row, int col);

protected:
    void fillTable();
    void fillComboBox();
    void setText(int row, int col, const QString& text);
    void adjustRows(int iRows);
    void adjustCols(int iCols);
    bool checkUpdateRange();
    QTextCodec* getCodec(void) const;

    // The real contents of the dialog
    KoCsvImportWidget *m_dialog;

    bool       m_adjustRows;
    bool       m_adjustCols;
    int        m_startRow;
    int        m_startCol;
    int        m_endRow;
    int        m_endCol;
    QChar      m_textquote;
    QString    m_delimiter;
    bool       m_ignoreDups;
    QByteArray m_fileArray;
    QTextCodec *m_codec;
    QStringList m_formatList; ///< List of the column formats

protected slots:
    void returnPressed();
    void formatChanged( const QString& );
    void delimiterClicked(int id);
    void textquoteSelected(const QString& mark);
    void currentCellChanged(int, int col);
    void textChanged ( const QString & );
    void ignoreDuplicatesChanged( int );
    void updateClicked();
    void encodingChanged ( const QString & );
};

#endif // KO_CSV_IMPORT_DIALOG

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

#include "KoCsvImportDialog.h"

// Qt
#include <QTextCodec>
#include <QTextStream>

#include <Q3Table>

// KDE
#include <kcharsets.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "ui_KoCsvImportDialog.h"

class KoCsvImportWidget : public QWidget, public Ui::KoCsvImportWidget
{
public:
    explicit KoCsvImportWidget(QWidget* parent) : QWidget(parent) { setupUi(this); }
};


class KoCsvImportDialog::Private
{
public:
    KoCsvImportWidget* dialog;

    bool        adjustRows;
    bool        adjustCols;
    int         startRow;
    int         startCol;
    int         endRow;
    int         endCol;
    QChar       textQuote;
    QString     delimiter;
    bool        ignoreDuplicates;
    QByteArray  data;
    QTextCodec* codec;
    QStringList formatList; ///< List of the column formats
};

KoCsvImportDialog::KoCsvImportDialog(QWidget* parent)
    : KDialog(parent)
    , d(new Private)
{
    d->dialog = new KoCsvImportWidget(this);
    d->adjustRows = false;
    d->adjustCols = false;
    d->startRow = 0;
    d->startCol = 0;
    d->endRow = -1;
    d->endCol = -1;
    d->textQuote = QChar('"');
    d->delimiter = QString(',');
    d->ignoreDuplicates = false;
    d->codec = QTextCodec::codecForName("UTF-8");

    setButtons( KDialog::Ok|KDialog::Cancel );
    setDefaultButton(KDialog::No);

    setCaption( i18n( "Import Data" ) );

    QStringList encodings;
    encodings << i18nc( "Descriptive encoding name", "Recommended ( %1 )" ,"UTF-8" );
    encodings << i18nc( "Descriptive encoding name", "Locale ( %1 )" ,QString(QTextCodec::codecForLocale()->name() ));
    encodings += KGlobal::charsets()->descriptiveEncodingNames();
    // Add a few non-standard encodings, which might be useful for text files
    const QString description(i18nc("Descriptive encoding name","Other ( %1 )"));
    encodings << description.arg("Apple Roman"); // Apple
    encodings << description.arg("IBM 850") << description.arg("IBM 866"); // MS DOS
    encodings << description.arg("CP 1258"); // Windows
    d->dialog->comboBoxEncoding->insertItems( 0, encodings );

    d->formatList << i18n( "Text" );
    d->formatList << i18n( "Number" );
    //d->formatList << i18n( "Currency" );
    //d->formatList << i18n( "Date" );
    d->formatList << i18n( "Decimal Comma Number" );
    d->formatList << i18n( "Decimal Point Number" );
    d->dialog->m_formatComboBox->insertItems( 0, d->formatList );

    d->dialog->m_sheet->setReadOnly( true );

    //resize(sizeHint());
    resize( 600, 400 ); // Try to show as much as possible of the table view
    setMainWidget(d->dialog);

    d->dialog->m_sheet->setSelectionMode( Q3Table::Multi );

    QButtonGroup* buttonGroup = new QButtonGroup( this );
    buttonGroup->addButton(d->dialog->m_radioComma, 0);
    buttonGroup->addButton(d->dialog->m_radioSemicolon, 1);
    buttonGroup->addButton(d->dialog->m_radioSpace, 2);
    buttonGroup->addButton(d->dialog->m_radioTab, 3);
    buttonGroup->addButton(d->dialog->m_radioOther, 4);

    connect(d->dialog->m_formatComboBox, SIGNAL(activated( const QString& )),
            this, SLOT(formatChanged( const QString& )));
    connect(buttonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(delimiterClicked(int)));
    connect(d->dialog->m_delimiterEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressed()));
    connect(d->dialog->m_delimiterEdit, SIGNAL(textChanged ( const QString & )),
            this, SLOT(genericDelimiterChanged( const QString & ) ));
    connect(d->dialog->m_comboQuote, SIGNAL(activated(const QString &)),
            this, SLOT(textquoteSelected(const QString &)));
    connect(d->dialog->m_sheet, SIGNAL(currentChanged(int, int)),
            this, SLOT(currentCellChanged(int, int)));
    connect(d->dialog->m_ignoreDuplicates, SIGNAL(stateChanged(int)),
            this, SLOT(ignoreDuplicatesChanged(int)));
    connect(d->dialog->m_updateButton, SIGNAL(clicked()),
            this, SLOT(updateClicked()));
    connect(d->dialog->comboBoxEncoding, SIGNAL(textChanged ( const QString & )),
            this, SLOT(encodingChanged ( const QString & ) ));
}


KoCsvImportDialog::~KoCsvImportDialog()
{
    delete d;
}


// ----------------------------------------------------------------
//                       public methods


void KoCsvImportDialog::setData( const QByteArray& data )
{
    d->data = data;
    fillTable();
}


bool KoCsvImportDialog::firstRowContainHeaders()
{
    return d->dialog->m_firstRowHeader->isChecked();
}


bool KoCsvImportDialog::firstColContainHeaders()
{
    return d->dialog->m_firstColHeader->isChecked();
}


int KoCsvImportDialog::rows()
{
    int rows = d->dialog->m_sheet->numRows();

    if ( d->endRow >= 0 )
	rows = d->endRow - d->startRow + 1;

    return rows;
}


int KoCsvImportDialog::cols()
{
    int cols = d->dialog->m_sheet->numCols();

    if ( d->endCol >= 0 )
	cols = d->endCol - d->startCol + 1;

    return cols;
}


QString KoCsvImportDialog::text(int row, int col)
{
    // Check for overflow.
    if ( row >= rows() || col >= cols())
	return QString();

    return d->dialog->m_sheet->text( row - d->startRow, col - d->startCol );
}


// ----------------------------------------------------------------


void KoCsvImportDialog::fillTable( )
{
    int row, column;
    bool lastCharDelimiter = false;
    enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD, S_END_OF_QUOTED_FIELD,
           S_MAYBE_NORMAL_FIELD, S_NORMAL_FIELD } state = S_START;

    QChar x;
    QString field;

    qApp->setOverrideCursor(Qt::WaitCursor);

    for (row = 0; row < d->dialog->m_sheet->numRows(); ++row)
        for (column = 0; column < d->dialog->m_sheet->numCols(); ++column)
            d->dialog->m_sheet->clearCell(row, column);

    int maxColumn = 1;
    row = column = 1;
    QTextStream inputStream(d->data, QIODevice::ReadOnly);
    kDebug(30501) <<"Encoding:" << d->codec->name();
    inputStream.setCodec( d->codec );

    const int delimiterLength = d->delimiter.size();
    bool lastCharWasCr = false; // Last character was a Carriage Return
    while (!inputStream.atEnd())
    {
        inputStream >> x; // read one char

        // ### TODO: we should perhaps skip all other control characters
        if ( x == '\r' )
        {
            // We have a Carriage Return, assume that its role is the one of a LineFeed
            lastCharWasCr = true;
            x = '\n'; // Replace by Line Feed
        }
        else if ( x == '\n' && lastCharWasCr )
        {
            // The end of line was already handled by the Carriage Return, so do nothing for this character
            lastCharWasCr = false;
            continue;
        }
        else if ( x == QChar( 0xc ) )
        {
            // We have a FormFeed, skip it
            lastCharWasCr = false;
            continue;
        }
        else
        {
            lastCharWasCr = false;
        }

        if ( column > maxColumn )
          maxColumn = column;
        switch (state)
        {
         case S_START :
            if (x == d->textQuote)
            {
                state = S_QUOTED_FIELD;
            }
            else if (!d->delimiter.isEmpty() && x == d->delimiter.at(0))
            {
                const int pos = inputStream.pos();
                QString xString = x + inputStream.read( delimiterLength - 1 );
                if ( xString == d->delimiter )
                {
                  if ((d->ignoreDuplicates == false) || (lastCharDelimiter == false))
                      column += delimiterLength;
                  lastCharDelimiter = true;
                }
                else
                {
                  // reset to old position
                  inputStream.seek( pos );
                }
            }
            else if (x == '\n')
            {
                ++row;
                column = 1;
                if ( row > ( d->endRow - d->startRow ) && d->endRow >= 0 )
                  break;
            }
            else
            {
                field += x;
                state = S_MAYBE_NORMAL_FIELD;
            }
            break;
         case S_QUOTED_FIELD :
            if (x == d->textQuote)
            {
                state = S_MAYBE_END_OF_QUOTED_FIELD;
            }
            else if (x == '\n')
            {
                setText(row - d->startRow, column - d->startCol, field);
                field.clear();

                ++row;
                column = 1;
                if ( row > ( d->endRow - d->startRow ) && d->endRow >= 0 )
                  break;

                state = S_START;
            }
            else
            {
                field += x;
            }
            break;
         case S_MAYBE_END_OF_QUOTED_FIELD :
            if (x == d->textQuote)
            {
                field += x;
                state = S_QUOTED_FIELD;
            }
            else if (!d->delimiter.isEmpty() && x == d->delimiter.at(0) || x == '\n')
            {
                setText(row - d->startRow, column - d->startCol, field);
                field.clear();
                if (x == '\n')
                {
                    ++row;
                    column = 1;
                    if ( row > ( d->endRow - d->startRow ) && d->endRow >= 0 )
                      break;
                }
                else
                {
                  const int pos = inputStream.pos();
                  QString xString = x + inputStream.read( delimiterLength - 1 );
                  if ( xString == d->delimiter )
                  {
                    if ((d->ignoreDuplicates == false) || (lastCharDelimiter == false))
                      column += delimiterLength;
                    lastCharDelimiter = true;
                  }
                  else
                  {
                  // reset to old position
                    inputStream.seek( pos );
                  }
                }
                state = S_START;
            }
            else
            {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
         case S_END_OF_QUOTED_FIELD :
            if (!d->delimiter.isEmpty() && x == d->delimiter.at(0) || x == '\n')
            {
                setText(row - d->startRow, column - d->startCol, field);
                field.clear();
                if (x == '\n')
                {
                    ++row;
                    column = 1;
                    if ( row > ( d->endRow - d->startRow ) && d->endRow >= 0 )
                      break;
                }
                else
                {
                  const int pos = inputStream.pos();
                  QString xString = x + inputStream.read( delimiterLength - 1 );
                  if ( xString == d->delimiter )
                  {
                    if ((d->ignoreDuplicates == false) || (lastCharDelimiter == false))
                      column += delimiterLength;
                    lastCharDelimiter = true;
                  }
                  else
                  {
                  // reset to old position
                    inputStream.seek( pos );
                  }
                }
                state = S_START;
            }
            else
            {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
         case S_MAYBE_NORMAL_FIELD :
            if (x == d->textQuote)
            {
                field.clear();
                state = S_QUOTED_FIELD;
                break;
            }
         case S_NORMAL_FIELD :
            if (!d->delimiter.isEmpty() && x == d->delimiter.at(0) || x == '\n')
            {
                setText(row - d->startRow, column - d->startCol, field);
                field.clear();
                if (x == '\n')
                {
                    ++row;
                    column = 1;
                    if ( row > ( d->endRow - d->startRow ) && d->endRow >= 0 )
                      break;
                }
                else
                {
                  const int pos = inputStream.pos();
                  QString xString = x + inputStream.read( delimiterLength - 1 );
                  if ( xString == d->delimiter )
                  {
                    if ((d->ignoreDuplicates == false) || (lastCharDelimiter == false))
                      column += delimiterLength;
                    lastCharDelimiter = true;
                  }
                  else
                  {
                  // reset to old position
                    inputStream.seek( pos );
                  }
                }
                state = S_START;
            }
            else
            {
                field += x;
            }
        }
        if (d->delimiter.isEmpty() || x != d->delimiter.at(0))
          lastCharDelimiter = false;
    }

    if ( !field.isEmpty() )
    {
      // the last line of the file had not any line end
      setText(row - d->startRow, column - d->startCol, field);
      ++row;
      field.clear();
    }

    d->adjustCols = true;
    adjustRows( row - d->startRow );
    adjustCols( maxColumn - d->startCol );
    d->dialog->m_colEnd->setMaximum( maxColumn );
    if ( d->endCol == -1 )
      d->dialog->m_colEnd->setValue( maxColumn );
    

    for (column = 0; column < d->dialog->m_sheet->numCols(); ++column)
    {
        const QString header = d->dialog->m_sheet->horizontalHeader()->label(column);
        if ( d->formatList.contains( header ) )
            d->dialog->m_sheet->horizontalHeader()->setLabel(column, i18n("Text"));

        d->dialog->m_sheet->adjustColumn(column);
    }
    fillComboBox();

    qApp->restoreOverrideCursor();
}

void KoCsvImportDialog::fillComboBox()
{
  if ( d->endRow == -1 )
    d->dialog->m_rowEnd->setValue( d->dialog->m_sheet->numRows() );
  else
    d->dialog->m_rowEnd->setValue( d->endRow );

  if ( d->endCol == -1 )
    d->dialog->m_colEnd->setValue( d->dialog->m_sheet->numCols() );
  else
    d->dialog->m_colEnd->setValue( d->endCol );

  d->dialog->m_rowEnd->setMinimum( 1 );
  d->dialog->m_colEnd->setMinimum( 1 );
  d->dialog->m_rowEnd->setMaximum( d->dialog->m_sheet->numRows() );
  d->dialog->m_colEnd->setMaximum( d->dialog->m_sheet->numCols() );

  d->dialog->m_rowStart->setMinimum( 1 );
  d->dialog->m_colStart->setMinimum( 1 );
  d->dialog->m_rowStart->setMaximum( d->dialog->m_sheet->numRows() );
  d->dialog->m_colStart->setMaximum( d->dialog->m_sheet->numCols() );
}

int KoCsvImportDialog::headerType(int col)
{
    QString header = d->dialog->m_sheet->horizontalHeader()->label(col);
    
    if (header == i18n("Text"))
        return TEXT;
    else if (header == i18n("Number"))
        return NUMBER;
    else if (header == i18n("Currency"))
        return CURRENCY;
    else if ( header == i18n( "Date" ) )
        return DATE;
    else if ( header == i18n( "Decimal Comma Number" ) )
        return COMMANUMBER;
    else if ( header == i18n( "Decimal Point Number" ) )
        return POINTNUMBER;
    else
        return TEXT; // Should not happen
}

void KoCsvImportDialog::setText(int row, int col, const QString& text)
{
    if ( row < 1 || col < 1 ) // skipped by the user
        return;

    if ( ( row > ( d->endRow - d->startRow ) && d->endRow > 0 ) || ( col > ( d->endCol - d->startCol ) && d->endCol > 0 ) )
      return;

    if ( d->dialog->m_sheet->numRows() < row )
    {
        d->dialog->m_sheet->setNumRows( row + 5000 ); /* We add 5000 at a time to limit recalculations */
        d->adjustRows = true;
    }

    if ( d->dialog->m_sheet->numCols() < col )
    {
        d->dialog->m_sheet->setNumCols( col );
        d->adjustCols = true;
    }

    d->dialog->m_sheet->setText( row - 1, col - 1, text );
}

/*
 * Called after the first fillTable() when number of rows are unknown.
 */
void KoCsvImportDialog::adjustRows(int iRows)
{
    if (d->adjustRows)
    {
        d->dialog->m_sheet->setNumRows( iRows );
        d->adjustRows = false;
    }
}

void KoCsvImportDialog::adjustCols(int iCols)
{
    if (d->adjustCols)
    {  
        d->dialog->m_sheet->setNumCols( iCols );
        d->adjustCols = false;

        if ( d->endCol == -1 )
        {
          if ( iCols > ( d->endCol - d->startCol ) )
            iCols = d->endCol - d->startCol;

          d->dialog->m_sheet->setNumCols( iCols );
        }
    }
}

void KoCsvImportDialog::returnPressed()
{
    if (d->dialog->m_radioOther->isChecked())
        return;

    d->delimiter = d->dialog->m_delimiterEdit->text();
    fillTable();
}

void KoCsvImportDialog::genericDelimiterChanged( const QString & )
{
    d->dialog->m_radioOther->setChecked ( true );
    delimiterClicked(d->dialog->m_radioOther->group()->id(d->dialog->m_radioOther)); // other
}

void KoCsvImportDialog::formatChanged( const QString& newValue )
{
    //kDebug(30501) <<"KoCsvImportDialog::formatChanged:" << newValue;
    for ( int i = 0; i < d->dialog->m_sheet->numSelections(); ++i )
    {
        Q3TableSelection select ( d->dialog->m_sheet->selection( i ) );
        for ( int j = select.leftCol(); j <= select.rightCol() ; ++j )
        {
            d->dialog->m_sheet->horizontalHeader()->setLabel( j, newValue );
        }
    }
}

void KoCsvImportDialog::delimiterClicked(int id)
{
    const QButtonGroup* group = d->dialog->m_radioComma->group();
    if (id == group->id(d->dialog->m_radioComma) )
        d->delimiter = ",";
    else if (id == group->id(d->dialog->m_radioOther))
        d->delimiter = d->dialog->m_delimiterEdit->text();
    else if (id == group->id(d->dialog->m_radioTab))
        d->delimiter = "\t";
    else if (id == group->id(d->dialog->m_radioSpace))
        d->delimiter = " ";
    else if (id == group->id(d->dialog->m_radioSemicolon))
        d->delimiter = ";";

    kDebug(30501) <<"Delimiter \"" << d->delimiter <<"\" selected.";
    fillTable();
}

void KoCsvImportDialog::textquoteSelected(const QString& mark)
{
    if (mark == i18n("None"))
        d->textQuote = 0;
    else
        d->textQuote = mark[0];

    fillTable();
}

void KoCsvImportDialog::updateClicked()
{
  if ( !checkUpdateRange() )
    return;

  d->startRow = d->dialog->m_rowStart->value() - 1;
  d->endRow   = d->dialog->m_rowEnd->value();

  d->startCol  = d->dialog->m_colStart->value() - 1;
  d->endCol    = d->dialog->m_colEnd->value();

  fillTable();
}

bool KoCsvImportDialog::checkUpdateRange()
{
  if ( ( d->dialog->m_rowStart->value() > d->dialog->m_rowEnd->value() )
       || ( d->dialog->m_colStart->value() > d->dialog->m_colEnd->value() ) )
  {
    KMessageBox::error( this, i18n( "Please check the ranges you specified. The start value must be lower than the end value." ) );
    return false;
  }

  return true;
}

void KoCsvImportDialog::currentCellChanged(int, int col)
{
    const QString header = d->dialog->m_sheet->horizontalHeader()->label(col);
    d->dialog->m_formatComboBox->setItemText( d->dialog->m_formatComboBox->currentIndex(), header );
}

void KoCsvImportDialog::ignoreDuplicatesChanged(int)
{
  if (d->dialog->m_ignoreDuplicates->isChecked())
    d->ignoreDuplicates = true;
  else
    d->ignoreDuplicates = false;
  fillTable();
}

QTextCodec* KoCsvImportDialog::getCodec(void) const
{
    const QString strCodec( KGlobal::charsets()->encodingForName( d->dialog->comboBoxEncoding->currentText() ) );
    kDebug(30501) <<"Encoding:" << strCodec;

    bool ok = false;
    QTextCodec* codec = QTextCodec::codecForName( strCodec.toUtf8() );

    // If QTextCodec has not found a valid encoding, so try with KCharsets.
    if ( codec )
    {
        ok = true;
    }
    else
    {
        codec = KGlobal::charsets()->codecForName( strCodec, ok );
    }

    // Still nothing?
    if ( !codec || !ok )
    {
        // Default: UTF-8
        kWarning(30502) << "Cannot find encoding:" << strCodec << endl;
        // ### TODO: what parent to use?
        KMessageBox::error( 0, i18n("Cannot find encoding: %1", strCodec ) );
        return 0;
    }

    return codec;
}

void KoCsvImportDialog::encodingChanged ( const QString & )
{
    QTextCodec* codec = getCodec();

    if ( codec )
    {
        d->codec = codec;
        fillTable();
    }
}

#include <KoCsvImportDialog.moc>

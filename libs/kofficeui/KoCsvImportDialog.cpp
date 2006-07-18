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

// Qt
#include <q3table.h>
#include <QCheckBox>
#include <qcursor.h>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <qtextstream.h>
#include <q3buttongroup.h>
#include <QPushButton>
#include <qradiobutton.h>
#include <qtextcodec.h>

// KDE
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kcharsets.h>

// local
#include "KoCsvImportDialog.h"

KoCsvImportDialog::KoCsvImportDialog(QWidget* parent)
    : KDialog(parent),
      m_dialog(new KoCsvImportWidget(this)),
      m_adjustRows(false),
      m_adjustCols(false),
      m_startRow(0),
      m_startCol(0),
      m_endRow(-1),
      m_endCol(-1),
      m_textquote('"'),
      m_delimiter(","),
      m_ignoreDups(false),
      m_fileArray(),
      m_codec( QTextCodec::codecForName( "UTF-8" ) )
{
    setButtons( KDialog::Ok|KDialog::Cancel );
    setDefaultButton(KDialog::No);

    setCaption( i18n( "Import Data" ) );
    kapp->restoreOverrideCursor();

    QStringList encodings;
    encodings << i18nc( "Descriptive encoding name", "Recommended ( %1 )" ,"UTF-8" );
    encodings << i18nc( "Descriptive encoding name", "Locale ( %1 )" ,QString(QTextCodec::codecForLocale()->name() ));
    encodings += KGlobal::charsets()->descriptiveEncodingNames();
    // Add a few non-standard encodings, which might be useful for text files
    const QString description(i18nc("Descriptive encoding name","Other ( %1 )"));
    encodings << description.arg("Apple Roman"); // Apple
    encodings << description.arg("IBM 850") << description.arg("IBM 866"); // MS DOS
    encodings << description.arg("CP 1258"); // Windows
    m_dialog->comboBoxEncoding->insertItems( 0, encodings );

    m_formatList << i18n( "Text" );
    m_formatList << i18n( "Number" );
    //m_formatList << i18n( "Currency" );
    //m_formatList << i18n( "Date" );
    m_formatList << i18n( "Decimal Comma Number" );
    m_formatList << i18n( "Decimal Point Number" );
    m_dialog->m_formatComboBox->insertItems( 0, m_formatList );

    m_dialog->m_sheet->setReadOnly( true );

    //resize(sizeHint());
    resize( 600, 400 ); // Try to show as much as possible of the table view
    setMainWidget(m_dialog);

    m_dialog->m_sheet->setSelectionMode( Q3Table::Multi );
    QButtonGroup* buttonGroup = m_dialog->m_radioComma->group();

    connect(m_dialog->m_formatComboBox, SIGNAL(activated( const QString& )),
            this, SLOT(formatChanged( const QString& )));
    connect(buttonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(delimiterClicked(int)));
    connect(m_dialog->m_delimiterEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressed()));
    connect(m_dialog->m_delimiterEdit, SIGNAL(textChanged ( const QString & )),
            this, SLOT(formatChanged ( const QString & ) ));
    connect(m_dialog->m_comboQuote, SIGNAL(activated(const QString &)),
            this, SLOT(textquoteSelected(const QString &)));
    connect(m_dialog->m_sheet, SIGNAL(currentChanged(int, int)),
            this, SLOT(currentCellChanged(int, int)));
    connect(m_dialog->m_ignoreDuplicates, SIGNAL(stateChanged(int)),
            this, SLOT(ignoreDuplicatesChanged(int)));
    connect(m_dialog->m_updateButton, SIGNAL(clicked()),
            this, SLOT(updateClicked()));
    connect(m_dialog->comboBoxEncoding, SIGNAL(textChanged ( const QString & )),
            this, SLOT(encodingChanged ( const QString & ) ));
}


KoCsvImportDialog::~KoCsvImportDialog()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
}


// ----------------------------------------------------------------
//                       public methods


void KoCsvImportDialog::setData( const QByteArray& data )
{
    m_fileArray = data;
    fillTable();
}


bool KoCsvImportDialog::firstRowContainHeaders()
{
    return m_dialog->m_firstRowHeader->isChecked();
}


bool KoCsvImportDialog::firstColContainHeaders()
{
    return m_dialog->m_firstColHeader->isChecked();
}


int KoCsvImportDialog::rows()
{
    int rows = m_dialog->m_sheet->numRows();

    if ( m_endRow >= 0 )
	rows = m_endRow - m_startRow + 1;

    return rows;
}


int KoCsvImportDialog::cols()
{
    int cols = m_dialog->m_sheet->numCols();

    if ( m_endCol >= 0 )
	cols = m_endCol - m_startCol + 1;

    return cols;
}


QString KoCsvImportDialog::text(int row, int col)
{
    // Check for overflow.
    if ( row >= rows() || col >= cols())
	return QString::null;

    return m_dialog->m_sheet->text( row - m_startRow, col - m_startCol );
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

    kapp->setOverrideCursor(Qt::WaitCursor);

    for (row = 0; row < m_dialog->m_sheet->numRows(); ++row)
        for (column = 0; column < m_dialog->m_sheet->numCols(); ++column)
            m_dialog->m_sheet->clearCell(row, column);

    int maxColumn = 1;
    row = column = 1;
    QTextStream inputStream(m_fileArray, QIODevice::ReadOnly);
    kDebug(30501) << "Encoding: " << m_codec->name() << endl;
    inputStream.setCodec( m_codec );

    const int delimiterLength = m_delimiter.size();
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
            if (x == m_textquote)
            {
                state = S_QUOTED_FIELD;
            }
            else if (x == m_delimiter.at(0))
            {
                const int pos = inputStream.pos();
                QString xString = x + inputStream.read( delimiterLength - 1 );
                if ( xString == m_delimiter )
                {
                  if ((m_ignoreDups == false) || (lastCharDelimiter == false))
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
                if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                  break;
            }
            else
            {
                field += x;
                state = S_MAYBE_NORMAL_FIELD;
            }
            break;
         case S_QUOTED_FIELD :
            if (x == m_textquote)
            {
                state = S_MAYBE_END_OF_QUOTED_FIELD;
            }
            else if (x == '\n')
            {
                setText(row - m_startRow, column - m_startCol, field);
                field = QString::null;

                ++row;
                column = 1;
                if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                  break;

                state = S_START;
            }
            else
            {
                field += x;
            }
            break;
         case S_MAYBE_END_OF_QUOTED_FIELD :
            if (x == m_textquote)
            {
                field += x;
                state = S_QUOTED_FIELD;
            }
            else if (x == m_delimiter.at(0) || x == '\n')
            {
                setText(row - m_startRow, column - m_startCol, field);
                field = QString::null;
                if (x == '\n')
                {
                    ++row;
                    column = 1;
                    if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                      break;
                }
                else
                {
                  const int pos = inputStream.pos();
                  QString xString = x + inputStream.read( delimiterLength - 1 );
                  if ( xString == m_delimiter )
                  {
                    if ((m_ignoreDups == false) || (lastCharDelimiter == false))
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
            if (x == m_delimiter.at(0) || x == '\n')
            {
                setText(row - m_startRow, column - m_startCol, field);
                field = QString::null;
                if (x == '\n')
                {
                    ++row;
                    column = 1;
                    if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                      break;
                }
                else
                {
                  const int pos = inputStream.pos();
                  QString xString = x + inputStream.read( delimiterLength - 1 );
                  if ( xString == m_delimiter )
                  {
                    if ((m_ignoreDups == false) || (lastCharDelimiter == false))
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
            if (x == m_textquote)
            {
                field = QString::null;
                state = S_QUOTED_FIELD;
                break;
            }
         case S_NORMAL_FIELD :
            if (x == m_delimiter.at(0) || x == '\n')
            {
                setText(row - m_startRow, column - m_startCol, field);
                field = QString::null;
                if (x == '\n')
                {
                    ++row;
                    column = 1;
                    if ( row > ( m_endRow - m_startRow ) && m_endRow >= 0 )
                      break;
                }
                else
                {
                  const int pos = inputStream.pos();
                  QString xString = x + inputStream.read( delimiterLength - 1 );
                  if ( xString == m_delimiter )
                  {
                    if ((m_ignoreDups == false) || (lastCharDelimiter == false))
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
        if (x != m_delimiter.at(0))
          lastCharDelimiter = false;
    }

    if ( !field.isEmpty() )
    {
      // the last line of the file had not any line end
      setText(row - m_startRow, column - m_startCol, field);
      ++row;
      field = QString::null;
    }
    
    m_adjustCols = true;
    adjustRows( row - m_startRow );
    adjustCols( maxColumn - m_startCol );
    m_dialog->m_colEnd->setMaximum( maxColumn );
    if ( m_endCol == -1 )
      m_dialog->m_colEnd->setValue( maxColumn );
    

    for (column = 0; column < m_dialog->m_sheet->numCols(); ++column)
    {
        const QString header = m_dialog->m_sheet->horizontalHeader()->label(column);
        if ( m_formatList.contains( header ) )
            m_dialog->m_sheet->horizontalHeader()->setLabel(column, i18n("Text"));

        m_dialog->m_sheet->adjustColumn(column);
    }
    fillComboBox();

    kapp->restoreOverrideCursor();
}

void KoCsvImportDialog::fillComboBox()
{
  if ( m_endRow == -1 )
    m_dialog->m_rowEnd->setValue( m_dialog->m_sheet->numRows() );  
  else
    m_dialog->m_rowEnd->setValue( m_endRow );

  if ( m_endCol == -1 )
    m_dialog->m_colEnd->setValue( m_dialog->m_sheet->numCols() );
  else
    m_dialog->m_colEnd->setValue( m_endCol );  

  m_dialog->m_rowEnd->setMinimum( 1 );
  m_dialog->m_colEnd->setMinimum( 1 );
  m_dialog->m_rowEnd->setMaximum( m_dialog->m_sheet->numRows() );
  m_dialog->m_colEnd->setMaximum( m_dialog->m_sheet->numCols() );

  m_dialog->m_rowStart->setMinimum( 1 );
  m_dialog->m_colStart->setMinimum( 1 );
  m_dialog->m_rowStart->setMaximum( m_dialog->m_sheet->numRows() );
  m_dialog->m_colStart->setMaximum( m_dialog->m_sheet->numCols() );
}

int KoCsvImportDialog::headerType(int col)
{
    QString header = m_dialog->m_sheet->horizontalHeader()->label(col);
    
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

    if ( ( row > ( m_endRow - m_startRow ) && m_endRow > 0 ) || ( col > ( m_endCol - m_startCol ) && m_endCol > 0 ) )
      return;

    if ( m_dialog->m_sheet->numRows() < row ) 
    {
        m_dialog->m_sheet->setNumRows( row + 5000 ); /* We add 5000 at a time to limit recalculations */
        m_adjustRows = true;
    }

    if ( m_dialog->m_sheet->numCols() < col )
    {
        m_dialog->m_sheet->setNumCols( col );
        m_adjustCols = true;
    }

    m_dialog->m_sheet->setText( row - 1, col - 1, text );
}

/*
 * Called after the first fillTable() when number of rows are unknown.
 */
void KoCsvImportDialog::adjustRows(int iRows)
{
    if (m_adjustRows) 
    {
        m_dialog->m_sheet->setNumRows( iRows );
        m_adjustRows = false;
    }
}

void KoCsvImportDialog::adjustCols(int iCols)
{
    if (m_adjustCols) 
    {  
        m_dialog->m_sheet->setNumCols( iCols );
        m_adjustCols = false;

        if ( m_endCol == -1 )
        {
          if ( iCols > ( m_endCol - m_startCol ) )
            iCols = m_endCol - m_startCol;

          m_dialog->m_sheet->setNumCols( iCols );
        }
    }
}

void KoCsvImportDialog::returnPressed()
{
    if (m_dialog->m_radioOther->isChecked())
        return;

    m_delimiter = m_dialog->m_delimiterEdit->text();
    fillTable();
}

void KoCsvImportDialog::textChanged ( const QString & )
{
    m_dialog->m_radioOther->setChecked ( true );
    delimiterClicked(m_dialog->m_radioOther->group()->id(m_dialog->m_radioOther)); // other
}

void KoCsvImportDialog::formatChanged( const QString& newValue )
{
    //kDebug(30501) << "KoCsvImportDialog::formatChanged:" << newValue << endl;
    for ( int i = 0; i < m_dialog->m_sheet->numSelections(); ++i )
    {
        Q3TableSelection select ( m_dialog->m_sheet->selection( i ) );
        for ( int j = select.leftCol(); j <= select.rightCol() ; ++j )
        {
            m_dialog->m_sheet->horizontalHeader()->setLabel( j, newValue );
        }
    }
}

void KoCsvImportDialog::delimiterClicked(int id)
{
    const QButtonGroup* group = m_dialog->m_radioComma->group();
    if (id == group->id(m_dialog->m_radioComma) )
        m_delimiter = ",";
    else if (id == group->id(m_dialog->m_radioOther))
        m_delimiter = m_dialog->m_delimiterEdit->text();
    else if (id == group->id(m_dialog->m_radioTab))
        m_delimiter = "\t";
    else if (id == group->id(m_dialog->m_radioSpace))
        m_delimiter = " ";
    else if (id == group->id(m_dialog->m_radioSemicolon))
        m_delimiter = ";";

    fillTable();
}

void KoCsvImportDialog::textquoteSelected(const QString& mark)
{
    if (mark == i18n("None"))
        m_textquote = 0;
    else
        m_textquote = mark[0];

    fillTable();
}

void KoCsvImportDialog::updateClicked()
{
  if ( !checkUpdateRange() )
    return;

  m_startRow = m_dialog->m_rowStart->value() - 1;
  m_endRow   = m_dialog->m_rowEnd->value();

  m_startCol  = m_dialog->m_colStart->value() - 1;
  m_endCol    = m_dialog->m_colEnd->value();

  fillTable();
}

bool KoCsvImportDialog::checkUpdateRange()
{
  if ( ( m_dialog->m_rowStart->value() > m_dialog->m_rowEnd->value() ) 
       || ( m_dialog->m_colStart->value() > m_dialog->m_colEnd->value() ) )
  {
    KMessageBox::error( this, i18n( "Please check the ranges you specified. The start value must be lower than the end value." ) );
    return false;
  }

  return true;
}

void KoCsvImportDialog::currentCellChanged(int, int col)
{
    const QString header = m_dialog->m_sheet->horizontalHeader()->label(col);
    m_dialog->m_formatComboBox->setItemText( m_dialog->m_formatComboBox->currentIndex(), header );
}

void KoCsvImportDialog::ignoreDuplicatesChanged(int)
{
  if (m_dialog->m_ignoreDuplicates->isChecked())
    m_ignoreDups = true;
  else
    m_ignoreDups = false;
  fillTable();
}

QTextCodec* KoCsvImportDialog::getCodec(void) const
{
    const QString strCodec( KGlobal::charsets()->encodingForName( m_dialog->comboBoxEncoding->currentText() ) );
    kDebug(30502) << "Encoding: " << strCodec << endl;

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
        m_codec = codec;
        fillTable();
    }
}

#include <KoCsvImportDialog.moc>

/* This file is part of the KDE project
   Copyright (C) 2009 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2014 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "spreadsheetmigrate.h"

#include <kexiutils/identifier.h>

namespace KexiMigration
{

K_EXPORT_KEXIMIGRATE_DRIVER(SpreadsheetMigrate, spreadsheet)

//! a KoPart used to fill required part in spreadsheet Doc object
class MockPart : public KoPart
{
public:
    MockPart()
    : KoPart( 0 )
    {}
    KoView *createViewInstance(KoDocument* document, QWidget* parent) { Q_UNUSED(document); Q_UNUSED(parent); return 0; }
    virtual KoMainWindow *createMainWindow() { return 0; }
protected:
    virtual QGraphicsItem *createCanvasItem(KoDocument* document) { Q_UNUSED(document); return 0; }
};

// ---

SpreadsheetMigrate::SpreadsheetMigrate(QObject *parent, const QVariantList &args)
        : KexiMigrate(parent, args)
{
  m_CurSheet = 0;
  m_KSDoc = 0;
  m_Row = 0;
}

SpreadsheetMigrate::~SpreadsheetMigrate()
{
  if (m_KSDoc) {
    m_KSDoc->closeUrl();
    m_KSDoc->deleteLater();
  }
}

bool SpreadsheetMigrate::drv_connect()
{
  drv_disconnect();
  m_FileName = data()->source->dbPath() + '/' + data()->source->dbFileName();
  
  if (!QFile::exists(m_FileName)) 
    return false;
  
  if (!m_KSDoc) {
      m_KSDoc = new Calligra::Sheets::Doc(new MockPart);
  }
  kDebug();
  return m_KSDoc->openUrl(m_FileName);
}

bool SpreadsheetMigrate::drv_disconnect()
{
  if (m_KSDoc) {
    m_KSDoc->closeUrl();
    delete m_KSDoc;
    m_KSDoc = 0;
  }
  return true;
}

bool SpreadsheetMigrate::drv_tableNames(QStringList& tablenames)
{
    QList<Calligra::Sheets::Sheet*> sheets = m_KSDoc->map()->sheetList();
  
    kDebug() << sheets.size() << "sheets" << m_KSDoc->map()->sheetList().size();
  
    foreach(Calligra::Sheets::Sheet *sheet, sheets) {
        tablenames << sheet->sheetName();
    }
  
  return true;
}

bool SpreadsheetMigrate::drv_copyTable(const QString& srcTable, KexiDB::Connection *destConn,
                                       KexiDB::TableSchema* dstTable)
{
    Q_UNUSED(srcTable);
    Q_UNUSED(destConn);
    Q_UNUSED(dstTable);
    //! @todo implement data copying
    return true;
}

bool SpreadsheetMigrate::drv_readTableSchema(const QString& originalName, KexiDB::TableSchema& tableSchema)
{
  Calligra::Sheets::Sheet *sheet = m_KSDoc->map()->findSheet(originalName);
  
  if (!sheet)
  {
      kWarning() << "unable to find sheet" << originalName;
      return false;
  }
  
  int row=1, col = 1;
  
  Calligra::Sheets::Cell *cell;
  
  forever {
      cell = new Calligra::Sheets::Cell(sheet, col, row);
      if (cell->isEmpty()) {
          break;
      }
      QString fieldCaption = cell->displayText();
      // find unique field name
      QString fieldBaseName = KexiUtils::stringToIdentifier(fieldCaption).toLower();
      int fieldNameAdd = 0;
      QString fieldName;
      forever {
          fieldName = fieldBaseName;
          if (fieldNameAdd > 0) {
              fieldName.append(QLatin1Char('_') + QString::number(fieldNameAdd));
          }
          if (!tableSchema.field(fieldName)) {
              break;
          }
          fieldNameAdd++;
      }
      KexiDB::Field *fld = new KexiDB::Field(fieldName, KexiDB::Field::Text);
      fld->setCaption(fieldCaption);
      tableSchema.addField( fld );
      kDebug() << fieldName;
      col++;
  }
  
  return true;
}

bool SpreadsheetMigrate::drv_readFromTable(const QString & tableName)
{
  m_CurSheet = m_KSDoc->map()->findSheet(tableName);
  
  m_Row = 1;
  
  return m_CurSheet;
}

bool SpreadsheetMigrate::drv_moveNext()
{
  if (!m_CurSheet)
    return false;
  
  Calligra::Sheets::Cell cell = Calligra::Sheets::Cell(m_CurSheet, 1, m_Row + 1);
  
  if (!cell.isEmpty())
  {
    m_Row++;
    return true;
  }
  
  return false;
}

bool SpreadsheetMigrate::drv_movePrevious()
{
  if (!m_CurSheet)
    return false;
  
  if (m_Row > 1)
  {
    m_Row--;
    return true;
  }
  return false;
}

bool SpreadsheetMigrate::drv_moveFirst()
{
  if (!m_CurSheet)
    return false;
  
  m_Row = 1;
  return drv_moveNext();
}

bool SpreadsheetMigrate::drv_moveLast()
{
  if (!m_CurSheet)
    return false;
  
  while(drv_moveNext()){}
  
  return true;
}

QVariant SpreadsheetMigrate::drv_value(uint i)
{
    return Calligra::Sheets::Cell(m_CurSheet, i+1, m_Row).value().asVariant();
}

}

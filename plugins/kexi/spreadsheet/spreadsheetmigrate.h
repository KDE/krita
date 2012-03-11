/* This file is part of the KDE project
Copyright (C) 2009 Adam Pigg <adam@piggz.co.uk>

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

#ifndef KEXIMIGRATIONTXTMIGRATE_H
#define KEXIMIGRATIONTXTMIGRATE_H

#include <migration/keximigrate.h>
#include <migration/keximigrate_p.h>
#include <sheets/part/Doc.h>
#include <sheets/Map.h>
#include <sheets/Sheet.h>
#include <KoStore.h>
#include <KoOdfWriteStore.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoDocument.h>

namespace KexiMigration
{

/**
@author Adam Pigg
*/
class SpreadsheetMigrate : public KexiMigrate
{
    Q_OBJECT
    KEXIMIGRATION_DRIVER
public:
    SpreadsheetMigrate(QObject *parent, const QVariantList &args = QVariantList());

    virtual ~SpreadsheetMigrate();

  protected:
    //! Connect to source
    virtual bool drv_connect();

    //! Disconnect from source
    virtual bool drv_disconnect();

    //! Get table names in source
    virtual bool drv_tableNames(QStringList& tablenames);

    virtual bool drv_copyTable(const QString&, KexiDB::Connection*, KexiDB::TableSchema*){return false;};

    //! Read schema for a given table
    virtual bool drv_readTableSchema(const QString& originalName, KexiDB::TableSchema& tableSchema);

    //! Position the source dataset at the start of a table
    virtual bool drv_readFromTable(const QString & tableName);

    //! Move to the next row
    virtual bool drv_moveNext();

    //! Move to the previous row
    virtual bool drv_movePrevious();
    
    //! Move to the next row
    virtual bool drv_moveFirst();

    //! Move to the previous row
    virtual bool drv_moveLast();
    
    //! Read the data at the given row/field
    virtual QVariant drv_value(uint i);

  private:
    QString m_FileName;
    Calligra::Sheets::Sheet *m_CurSheet;
    
    Calligra::Sheets::Doc *m_KSDoc;

    QStringList m_FieldNames;

    unsigned long m_Row;

};

}

#endif

/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_RELATIONSHIP_H
#define KEXIDB_RELATIONSHIP_H

#include "field.h"

namespace KexiDB
{

/*! KexiDB::Relationship provides information about one-to-many relationship between two tables.
 Relationship is defined by a pair of (potentially multi-field) indices:
 - "one" or "master" side: unique key
 - "many" or "details" side: referenced foreign key
 <pre>
 [unique key, master] ----< [foreign key, details]
 </pre>

 In this documentation, we will call table that owns fields of "one" side as
 "master side of the relationship", and the table that owns foreign key fields of
 as "details side of the relationship".
 Use masterTable(), and detailsTable() to get one-side table and many-side table, respectively.

 Note: some engines (e.g. MySQL with InnoDB) requires that indices at both sides
 have to be explicitly created.

 \todo (js) It is planned that this will be handled by KexiDB internally and transparently.

 Each (of the two) key can be defined (just like index) as list of fields owned by one table.
 Indeed, relationship info can retrieved from Relationship object in two ways:
 -# pair of indices; use masterIndex(), detailsIndex() for that
 -# ordered list of field pairs (<master-side-field, details-side-field>); use fieldPairs() for that

 No assigned objects (like fields, indices) are owned by Relationship object. The exception is that
 list of field-pairs is internally created (on demand) and owned.

 Relationship object is owned by IndexSchema object (the one that is defined at master-side of the
 relationship).
 Note also that IndexSchema objects are owned by appropriate tables, so thus
 there is implicit ownership between TableSchema and Relationship.

 If Relationship object is not attached to IndexSchema object,
 you should care about destroying it by hand.

  Example:
  <pre>
            ----------
   ---r1--<|          |
           | Table A [uk]----r3---<
   ---r2--<|          |
            ----------
  </pre>
  Table A has two relationships (r1, r2) at details side and one (r3) at master side.
  [uk] stands for unique key.
*/

class IndexSchema;
class TableSchema;
class QuerySchema;

class CALLIGRADB_EXPORT Relationship
{
public:
    typedef QList<Relationship*> List;
    typedef QList<Relationship*>::ConstIterator ListIterator;

    /*! Creates uninitialized Relationship object.
      setIndices() will be required to call.
    */
    Relationship();

    /*! Creates Relationship object and initialises it just by
     calling setIndices(). If setIndices() failed, object is still uninitialised.
    */
    Relationship(IndexSchema* masterIndex, IndexSchema* detailsIndex);

    virtual ~Relationship();

    /*! \return index defining master side of this relationship
     or null if there is no information defined. */
    IndexSchema* masterIndex() const {
        return m_masterIndex;
    }

    /*! \return index defining referenced side of this relationship.
     or null if there is no information defined. */
    IndexSchema* detailsIndex() const {
        return m_detailsIndex;
    }

    /*! \return ordered list of field pairs -- alternative form
     for representation of relationship or null if there is no information defined.
     Each pair has a form of <master-side-field, details-side-field>. */
    Field::PairList* fieldPairs() {
        return &m_pairs;
    }

    bool isEmpty() const {
        return m_pairs.isEmpty();
    }

    /*! \return table assigned at "master / one" side of this relationship.
     or null if there is no information defined. */
    TableSchema* masterTable() const;

    /*! \return table assigned at "details / many / foreign" side of this relationship.
     or null if there is no information defined. */
    TableSchema* detailsTable() const;

    /*! Sets \a masterIndex and \a detailsIndex indices for this relationship.
     This also sets information about tables for master- and details- sides.
     Notes:
     - both indices must contain the same number of fields
     - both indices must not be owned by the same table, and table (owner) must be not null.
     - corresponding field types must be the same
     - corresponding field types' signedness must be the same
     If above rules are not fulfilled, information about this relationship is cleared.
     On success, this Relationship object is detached from previous IndexSchema objects that were
     assigned before, and new are attached.
     */
    void setIndices(IndexSchema* masterIndex, IndexSchema* detailsIndex);

protected:
    Relationship(QuerySchema *query, Field *field1, Field *field2);

    void createIndices(QuerySchema *query, Field *field1, Field *field2);

    /*! Internal version of setIndices(). \a ownedByMaster parameter is passed
     to IndexSchema::attachRelationship() */
    void setIndices(IndexSchema* masterIndex, IndexSchema* detailsIndex, bool ownedByMaster);

    IndexSchema *m_masterIndex;
    IndexSchema *m_detailsIndex;

    Field::PairList m_pairs;

    bool m_masterIndexOwned : 1;
    bool m_detailsIndexOwned : 1;

    friend class Connection;
    friend class TableSchema;
    friend class QuerySchema;
    friend class IndexSchema;
};

} //namespace KexiDB

#endif

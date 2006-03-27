/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#ifndef koStoreDevice_h
#define koStoreDevice_h

#include <KoStore.h>

/**
 * This class implements a QIODevice around KoStore, so that
 * it can be used to create a QDomDocument from it, to be written or read
 * using QDataStream or to be written using QTextStream
 */
class KoStoreDevice : public QIODevice
{
public:
  /// Note: KoStore::open() should be called before calling this.
  KoStoreDevice( KoStore * store ) : m_store(store) {
      setType( IO_Direct );
  }
  ~KoStoreDevice() {}

  bool open( int m ) {
    if ( m & QIODevice::ReadOnly )
      return ( m_store->mode() == KoStore::Read );
    if ( m & QIODevice::WriteOnly )
      return ( m_store->mode() == KoStore::Write );
    return false;
  }
  void close() { }
  void flush() { }

  Offset size() const {
    if ( m_store->mode() == KoStore::Read )
      return m_store->size();
    else
      return 0xffffffff;
  }

  virtual Q_LONG readBlock( char *data, Q_ULONG maxlen ) { return m_store->read(data, maxlen); }
  virtual Q_LONG writeBlock( const char *data, Q_ULONG len ) { return m_store->write( data, len ); }
  // Not virtual, only to uncover shadow
  Q_LONG writeBlock( const QByteArray& data ) { return QIODevice::writeBlock( data ); }

  int getch() {
    char c[2];
    if ( m_store->read(c, 1) == -1)
      return -1;
    else
      return c[0];
  }
  int putch( int _c ) {
    char c[2];
    c[0] = _c;
    c[1] = 0;
    if (m_store->write( c, 1 ) == 1)
      return _c;
    else
      return -1;
  }
  int ungetch( int ) { return -1; } // unsupported

  // See QIODevice
  virtual bool at( Offset pos ) { return m_store->at(pos); }
  virtual Offset at() const { return m_store->at(); }
  virtual bool atEnd() const { return m_store->atEnd(); }

protected:
  KoStore * m_store;
};

#endif

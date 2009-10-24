/*  POLE - Portable C++ library to access OLE Storage
    Copyright (C) 2002-2007 Ariya Hidayat (ariya@kde.org).

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POLE_H
#define POLE_H

#include <string>
#include <list>

namespace POLE
{

class StorageIO;
class Stream;
class StreamIO;

class Storage
{
  friend class Stream;
  friend class StreamOut;

public:

  // for Storage::result()
  enum { Ok, OpenFailed, NotOLE, BadOLE, UnknownError };

  /**
   * Constructs a storage with name filename.
   **/
  Storage( const char* filename );

  /**
   * Destroys the storage.
   **/
  ~Storage();

  /**
   * Opens the storage. Returns true if no error occurs.
   **/
  bool open();

  /**
   * Closes the storage.
   **/
  void close();

  /**
   * Returns the error code of last operation.
   **/
  int result();

  /**
   * Finds all stream and directories in given path.
   **/
  std::list<std::string> entries( const std::string& path = "/" );

  /**
   * Returns true if specified entry name is a directory.
   */
  bool isDirectory( const std::string& name );

  /**
   * Finds and returns a stream with the specified name.
   * If reuse is true, this function returns the already created stream
   * (if any). Otherwise it will create the stream.
   *
   * When errors occur, this function returns NULL.
   *
   * You do not need to delete the created stream, it will be handled
   * automatically.
   **/
  Stream* stream( const std::string& name, bool reuse = true );
  //Stream* stream( const std::string& name, int mode = Stream::ReadOnly, bool reuse = true );

private:
  StorageIO* io;

  // no copy or assign
  Storage( const Storage& );
  Storage& operator=( const Storage& );

};

class Stream
{
  friend class Storage;
  friend class StorageIO;

public:

  /**
   * Creates a new stream.
   */
  // name must be absolute, e.g "/Workbook"
  Stream( Storage* storage, const std::string& name );

  /**
   * Destroys the stream.
   */
  ~Stream();

  /**
   * Returns the full stream name.
   */
  std::string fullName();

  /**
   * Returns the stream size.
   **/
  unsigned long size();

  /**
   * Returns the current read/write position.
   **/
  unsigned long tell();

  /**
   * Sets the read/write position.
   **/
  void seek( unsigned long pos );

  /**
   * Reads a byte.
   **/
  int getch();

  /**
   * Reads a block of data.
   **/
  unsigned long read( unsigned char* data, unsigned long maxlen );

  /**
   * Returns true if the read/write position is past the file.
   **/
  bool eof();

  /**
   * Returns true whenever error occurs.
   **/
  bool fail();

private:
  StreamIO* io;

  // no copy or assign
  Stream( const Stream& );
  Stream& operator=( const Stream& );
};

}

#endif // POLE_H

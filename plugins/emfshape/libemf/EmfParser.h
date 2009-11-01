/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ENHMETAFILE_H
#define ENHMETAFILE_H

#include "emf_export.h"

#include "EmfOutput.h"

#include <QString>
#include <QRect> // also provides QSize

/**
   \file

   Primary definitions for EMF parser
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace Libemf
{

/**
    %Parser for an EMF format file
 */
class EMF_EXPORT Parser
{
public:
    Parser();
    ~Parser();

    /**
     * Load an EMF file
     *
     * \param fileName the name of the file to load
     *
     * \return true on successful load, or false on failure
     */
    bool load( const QString &fileName );

    /**
     * Load an EMF file from a stream
     *
     * \param stream the stream to read from
     *
     * \return true on successful load, or false on failure
     */
    bool loadFromStream( QDataStream &stream );

    /**
       Set the output strategy for the parserr

       \param output pointer to a strategy implementation
    */
    void setOutput( AbstractOutput *output );

private:
    // read a single EMF record
    bool readRecord( QDataStream &stream );

    // temporary function to soak up unneeded bytes
    void soakBytes( QDataStream &stream, int numBytes );

    // temporary function to dump output bytes
    void outputBytes( QDataStream &stream, int numBytes );

    // Pointer to the output strategy
    AbstractOutput *mOutput;
};

}

#endif

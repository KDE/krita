/**
 * @brief PFS library - core API interfaces
 *
 * Classes for reading and writing a stream of PFS frames.
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Rafal Mantiuk and Grzegorz Krawczyk
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 * 
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id$
 */

#if !defined(_MSC_VER) && !defined(_MATLAB_VER)
#include <config.h>
#endif

#if defined(_MSC_VER) || defined(__CYGWIN__)
#include <io.h>
#define HAVE_SETMODE
#endif

#include <fcntl.h>

#include <string.h>
#include <assert.h>
#include <string>
#include <list>

#include <map>

#include "pfs.h"

#define PFSEOL "\x0a"
#define PFSEOLCH '\x0a'

#define MAX_RES 65535

using namespace std;

namespace pfs
{

const char *PFSFILEID="PFS1\x0a";


//------------------------------------------------------------------------------
// TagContainer implementation  
//------------------------------------------------------------------------------

typedef list<string> TagList;

class TagIteratorImpl: public TagIterator
{
  TagList::const_iterator it;
  const TagList &tagList;
  string tagName;
  
public:
  TagIteratorImpl( const TagList &tagList ) : tagList( tagList )
  {
    it = tagList.begin();
  }
  
  /**
   * Get next item on the list.
   */
  const char *getNext()
  {
    const string &tag = *(it++);
    size_t equalSign = tag.find( '=' );
    assert( equalSign != -1 );
    tagName = string( tag, 0, equalSign );
    return tagName.c_str();
  }
  
  /**
   * Returns true if there is still an item left on the list.
   */
  bool hasNext() const
  {
    return it != tagList.end();
  }
  
};

class TagContainerImpl: public TagContainer
{
public:
private:
  
  TagList tagList;

public:

//   ~TagContainerImpl()
//   {
//     tagList.clear();
//   }
  
  TagList::const_iterator tagsBegin() const
  {
    return tagList.begin();
  }

  TagList::const_iterator tagsEnd() const
  {
    return tagList.end();
  }

  int getSize() const
  {
    return (int)tagList.size();
  }

  void appendTagEOL( const char *tagValue )
  {
    assert( tagValue[strlen( tagValue ) -1] == PFSEOLCH );
    tagList.push_back( string( tagValue, strlen( tagValue ) -1 ) );
  }

  void appendTag( const string &tagValue )
  {
    tagList.push_back( tagValue );
  }
  
  TagList::iterator findTag( const char *tagName )
  {
    size_t tagNameLen = strlen( tagName );
    TagList::iterator it;
    for( it = tagList.begin(); it != tagList.end(); it++ ) {
      if( !memcmp( tagName, it->c_str(), tagNameLen ) ) break; // Found
    }
    return it;
  }

  void setTag( const char *tagName, const char *tagValue )
  {
    string tagVal( tagName );
    tagVal += "=";
    tagVal += tagValue;

    TagList::iterator element = findTag( tagName );
    if( element == tagList.end() ) { // Does not exist
      tagList.push_back( tagVal );
    } else {                // Already exist
      *element = tagVal;
    }
  }

  const char *getTag( const char *tagName )
  {
    TagList::iterator element = findTag( tagName );
    if( element == tagList.end() ) return NULL;

    string::size_type equalSign = element->find( '=' );
    assert( equalSign != string::npos );

    return element->c_str() + equalSign + 1;
  }


  //Implementation of TagContainer
  const char* getString( const char *tagName )
  {
    return getTag( tagName );
  }

  void setString( const char *tagName, const char *tagValue )
  {
    setTag( tagName, tagValue );
  }

  void removeTag( const char *tagName )
  {
    TagList::iterator element = findTag( tagName );
    if( element != tagList.end() ) tagList.erase( element );
  }

  TagIteratorPtr getIterator() const
  {
    return TagIteratorPtr( new TagIteratorImpl( tagList ) );
  }
  
  void removeAllTags()
  {
    tagList.clear();
  }
  

};

void copyTags( const TagContainer *from, TagContainer *to )
{
  TagContainerImpl *f = (TagContainerImpl*)from;
  TagContainerImpl *t = (TagContainerImpl*)to;

  t->removeAllTags();
  
  TagList::const_iterator it;
  for( it = f->tagsBegin(); it != f->tagsEnd(); it++ ) {
    t->appendTag( *it );
  }
}

void copyTags( Frame *from, Frame *to )
{
  copyTags( from->getTags(), to->getTags() );
  pfs::ChannelIterator *it = from->getChannels();
  while( it->hasNext() ) {
    pfs::Channel *fromCh = it->getNext();
    pfs::Channel *toCh = to->getChannel( fromCh->getName() );
    if( toCh == NULL ) // Skip if there is no corresponding channel
      continue;
    copyTags( fromCh->getTags(), toCh->getTags() );
  }  
  
}


//------------------------------------------------------------------------------
// Channel implementation  
//------------------------------------------------------------------------------

class DOMIOImpl;

class ChannelImpl: public Channel {
  int width, height;
  float *data;
  const char *name;

protected:
  friend class DOMIOImpl;

  TagContainerImpl *tags;

public:
  ChannelImpl( int width, int height, const char *n_name ) : width( width ), height( height )
  {
    data = new float[width*height];
    tags = new TagContainerImpl();
    name = strdup( n_name );
  }

  virtual ~ChannelImpl()
  {
    delete tags;
    delete[] data;
    free( (void*)name );
  }

  // Channel implementation
  TagContainer *getTags()
  {
    return tags;
  }

  float *getRawData()
  {
    return data;
  }

  //Array2D implementation

  virtual int getCols() const {
    return width;
  }

  virtual int getRows() const {
    return height;
  }

  virtual const char *getName() const
  {
    return name;
  }  
  
  inline float& operator()( int x, int y ) {
    assert( x >= 0 && x < width );
    assert( y >= 0 && y < height );
    return data[ x+y*width ];
  }

  inline const float& operator()( int x, int y ) const
  {
    assert( x >= 0 && x < width );
    assert( y >= 0 && y < height );
    return data[ x+y*width ];
  }

  inline float& operator()( int rowMajorIndex )
  {
    assert( rowMajorIndex < width*height );    
    assert( rowMajorIndex >= 0 );    
    return data[ rowMajorIndex ];
  }
  
  inline const float& operator()( int rowMajorIndex ) const
  {
    assert( rowMajorIndex < width*height );    
    assert( rowMajorIndex >= 0 );    
    return data[ rowMajorIndex ];
  }

};


//------------------------------------------------------------------------------
// Map of channels
//------------------------------------------------------------------------------

struct str_cmp: public std::binary_function<const char*,const char*,bool>
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};
typedef std::map<const char*, ChannelImpl*, str_cmp> ChannelMap;

//------------------------------------------------------------------------------
// Channel Iterator implementation
//-----------------------------------------------------------------------------

class ChannelIteratorImpl: public ChannelIterator
{
  ChannelMap::iterator it;
  ChannelMap *cm;
public:
  ChannelIteratorImpl( ChannelMap *cm ) : cm(cm)
  {
    reset();
  }

  void reset()
  {
    it = cm->begin();    
  }
  
  Channel *getNext() 
  {
    if( !hasNext() ) return NULL;
    return (it++)->second;
  }
    
  bool hasNext() const
  {
    return it != cm->end();
  }
    
};

//------------------------------------------------------------------------------
// Frame implementation  
//------------------------------------------------------------------------------

//A pure virtual destructor
Frame::~Frame()
  {}

class FrameImpl: public Frame {
  int width, height;

protected:
  friend class DOMIOImpl;

  TagContainerImpl *tags;

//  enum ChannelID { CH_X = 0, CH_Y, CH_Z, CH_ALPHA, CH_DEPTH, CH_AUXLUM, CH_COUNT };
//  static const char* const channelStrID[CH_COUNT];

  ChannelMap channel;

  ChannelIteratorImpl channelIterator;

public:

  FrameImpl( int width, int height ): width( width ), height( height ),
    channelIterator( &channel )
  {
    tags = new TagContainerImpl();
  }

  ~FrameImpl()
  {
    delete tags;
    ChannelMap::iterator it;
    for( it = channel.begin(); it != channel.end(); ) {
      Channel *ch = it->second;
      ChannelMap::iterator itToDelete = it; // Nasty trick because hashmap
                                            // elements point to string that is
                                            // freed by the channel 
      
      it++;
      channel.erase( itToDelete );
      delete ch;
    }
    
  }

  virtual int getWidth() const
  {
    return width;
  }

  virtual int getHeight() const
  {
    return height;
  }

  virtual void getXYZChannels( Channel* &X, Channel* &Y, Channel* &Z ) {

    if( channel.find("X") == channel.end() ||
      channel.find("Y") == channel.end() ||
      channel.find("Z") == channel.end() ) {
      X = Y = Z = NULL;
    } else {
      X = channel["X"];
      Y = channel["Y"];
      Z = channel["Z"];
    }
  }

  virtual void createXYZChannels( Channel* &X, Channel* &Y, Channel* &Z )
  {
    X = createChannel("X");
    Y = createChannel("Y");
    Z = createChannel("Z");
  }
  
  Channel* getChannel( const char *name )
  {
    ChannelMap::iterator it = channel.find(name);
    if( it == channel.end() )
      return NULL;
    else
      return it->second;
    
  }
  
  Channel *createChannel( const char *name )
  {
    ChannelImpl *ch;
    if( channel.find(name) == channel.end() ) {
      ch = new ChannelImpl( width, height, name );
      channel.insert( pair<const char*, ChannelImpl*>(ch->getName(), ch) );
    } else
      ch = channel[name];
    
    return ch;
  }

  void removeChannel( Channel *ch )
  {
    assert( ch != NULL );
    ChannelMap::iterator it = channel.find( ch->getName() );
    assert( it != channel.end() && it->second == ch );
    
    channel.erase( it );
    delete ch;
  }
  
  ChannelIterator *getChannels()
  {
    channelIterator.reset();
    return &channelIterator;
  }

  ChannelIteratorPtr getChannelIterator()
  {
    return ChannelIteratorPtr( new ChannelIteratorImpl( &channel ) );
  }

  TagContainer *getTags()
  {
    return tags;
  }


};

static void readTags( TagContainerImpl *tags, FILE *in )
{
  int readItems;
  int tagCount;
  readItems = fscanf( in, "%d" PFSEOL, &tagCount );
  if( readItems != 1 || tagCount < 0 || tagCount > 1024 )
    throw Exception( "Corrupted PFS tag section: missing or wrong number of tags" );

  char buf[1024];
  for( int i = 0; i < tagCount; i++ ) {
    char *read = fgets( buf, 1024, in );
    if( read == NULL ) throw Exception( "Corrupted PFS tag section: missing tag" );
    char *equalSign = strstr( buf, "=" );
    if( equalSign == NULL ) throw Exception( "Corrupted PFS tag section ('=' sign missing)" );
    tags->appendTagEOL( buf );
  }
}

static void writeTags( const TagContainerImpl *tags, FILE *out )
{
  TagList::const_iterator it;
  fprintf( out, "%d" PFSEOL, tags->getSize() );
  for( it = tags->tagsBegin(); it != tags->tagsEnd(); it++ ) {
    fprintf( out, it->c_str() );
    fprintf( out, PFSEOL );
  }
}



//------------------------------------------------------------------------------
// pfs IO
//------------------------------------------------------------------------------

class DOMIOImpl {
public:

  Frame *readFrame( FILE *inputStream )
  {
    assert( inputStream != NULL );
    
#ifdef HAVE_SETMODE
    // Needed under MS windows (text translation IO for stdin/out)
    int old_mode = setmode( fileno( inputStream ), _O_BINARY );
#endif

    size_t read;

    char buf[5];
    read = fread( buf, 1, 5, inputStream );
    if( read == 0 ) return NULL; // EOF

    if( memcmp( buf, PFSFILEID, 5 ) ) throw Exception( "Incorrect PFS file header" );

    int width, height, channelCount;
    read = fscanf( inputStream, "%d %d" PFSEOL, &width, &height );
    if( read != 2 || width <= 0 || width > MAX_RES || height <= 0 || height > MAX_RES )
      throw Exception( "Corrupted PFS file: missing or wrong 'width', 'height' tags" );
    read = fscanf( inputStream, "%d" PFSEOL, &channelCount );
    if( read != 1 || channelCount < 0 || channelCount > 1024 )
      throw Exception( "Corrupted PFS file: missing or wrong 'channelCount' tag" );

    FrameImpl *frame = (FrameImpl*)createFrame( width, height );

    readTags( frame->tags, inputStream );

    //Read channel IDs and tags
    //       FrameImpl::ChannelID *channelID = new FrameImpl::ChannelID[channelCount];
    list<ChannelImpl*> orderedChannel;
    for( int i = 0; i < channelCount; i++ ) {
      char channelName[10], *rs;
      rs = fgets( channelName, 10, inputStream );
      if( rs == NULL ) 
        throw Exception( "Corrupted PFS file: missing channel name" );
      size_t len = strlen( channelName );
//      fprintf( stderr, "s = '%s' len = %d\n", channelName, len );      
      if( len < 1 || channelName[len-1] != PFSEOLCH ) 
        throw Exception( "Corrupted PFS file: bad channel name" );
      channelName[len-1] = 0;
      ChannelImpl *ch = (ChannelImpl*)frame->createChannel( channelName );
      readTags( ch->tags, inputStream );
      orderedChannel.push_back( ch );
    }

    read = fread( buf, 1, 4, inputStream );
    if( read == 0 || memcmp( buf, "ENDH", 4 ) )
      throw Exception( "Corrupted PFS file: missing end of header (ENDH) token" );
    

    //Read channels
    list<ChannelImpl*>::iterator it;
    for( it = orderedChannel.begin(); it != orderedChannel.end(); it++ ) {
      ChannelImpl *ch = *it;
      int size = frame->getWidth()*frame->getHeight();
      read = fread( ch->getRawData(), sizeof( float ), size, inputStream );
      if( read != size )
        throw Exception( "Corrupted PFS file: missing channel data" );
    }
#ifdef HAVE_SETMODE
    setmode( fileno( inputStream ), old_mode );
#endif
    return frame;
  }


  Frame *createFrame( int width, int height )
  {
/*    if( lastFrame != NULL && lastFrame->width() == width && lastFrame->height() == height ) {
// Reuse last frame
return lastFrame;
} else
delete lastFrame;*/

    Frame *frame = new FrameImpl( width, height );
    if( frame == NULL ) throw Exception( "Out of memory" );
    return frame;
  }


  void writeFrame( Frame *frame, FILE *outputStream )
  {
    assert( outputStream != NULL );
    assert( frame != NULL );
#ifdef HAVE_SETMODE
    // Needed under MS windows (text translation IO for stdin/out)
    int old_mode = setmode( fileno( outputStream ), _O_BINARY );
#endif
    
    FrameImpl *frameImpl = (FrameImpl*)frame;

    fwrite( PFSFILEID, 1, 5, outputStream ); // Write header ID
    
    fprintf( outputStream, "%d %d" PFSEOL, frame->getWidth(), frame->getHeight() );
    fprintf( outputStream, "%d" PFSEOL, frameImpl->channel.size() );

    writeTags( frameImpl->tags, outputStream );

    //Write channel IDs and tags
    for( ChannelMap::iterator it = frameImpl->channel.begin(); it != frameImpl->channel.end(); it++ ) {
      fprintf( outputStream, "%s" PFSEOL, it->second->getName() );
      writeTags( it->second->tags, outputStream );
    }

    fprintf( outputStream, "ENDH");
    
    //Write channels
	{
    for( ChannelMap::iterator it = frameImpl->channel.begin(); it != frameImpl->channel.end(); it++ ) {
        int size = frame->getWidth()*frame->getHeight();
        fwrite( it->second->getRawData(), sizeof( float ), size, outputStream );
      }
	}

    //Very important for pfsoutavi !!!
    fflush(outputStream);
#ifdef HAVE_SETMODE
    setmode( fileno( outputStream ), old_mode );
#endif
  }

  void freeFrame( Frame *frame )
  {
    delete frame;
  }

};


DOMIO::DOMIO()
{
  impl = new DOMIOImpl();
}

DOMIO::~DOMIO()
{
  delete impl;
}

Frame *DOMIO::createFrame( int width, int height )
{
  return impl->createFrame( width, height );
}


Frame *DOMIO::readFrame( FILE *inputStream )
{
  return impl->readFrame( inputStream );
}


void DOMIO::writeFrame( Frame *frame, FILE *outputStream )
{
  impl->writeFrame( frame, outputStream );
}


void DOMIO::freeFrame( Frame *frame )
{
  impl->freeFrame( frame );
}

};

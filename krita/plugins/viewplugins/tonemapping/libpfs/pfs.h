/**
 * @file
 * @brief PFS library - core API interfaces
 *
 * Classes for reading and writing a stream of PFS frames.
 *
 * Note on the design of pfs library API: pfs library API makes
 * extensive usage of interfaces - classes that have only virtual
 * methods. This way no private fields are visible for the client
 * programs. Everything that is private is hidden in .cpp file rather
 * than the header .h. For example, pfs library uses STL to store some
 * internal data, but no STL class can be found the header file
 * pfs.h. Such design should hopefully give less problems when
 * extending and updating the library.
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

#ifndef PFS_H
#define PFS_H

#include <stdio.h>
#include <string.h>
#include "array2d.h"

struct option;

/**
 * All classes and function from PFS library reside in pfs namespace. 
 */
namespace pfs
{

  /**
   * Utility class that keeps pointer and deletes pointed object
   * when the class is deleted.
   *
   * Note that it is not a full implementation of the smart pointer
   * and memory management is not fool proof. You should never store
   * this object as a global variable or a field of a class. These
   * objects should be used only as local variables.
   */
  template<class T>
    class SelfDestructPtr
    {
      T *ptr;
      mutable bool itsOwn;
    public:
      explicit SelfDestructPtr( T *ptr = 0 ): ptr(ptr), itsOwn(ptr!=0)
        {
        }

      SelfDestructPtr( const SelfDestructPtr& r ) 
        : itsOwn(r.itsOwn), ptr(r.release()) {}
      
      SelfDestructPtr& operator=( const SelfDestructPtr& r ) {
        if (&r != this) {
            if (ptr != r.ptr) {
              if( itsOwn ) delete ptr;
              itsOwn = r.itsOwn;
            }
            else if( r.itsOwn ) itsOwn = true;
            ptr = r.release();
        }
        return *this;
      }
      
      ~SelfDestructPtr()
        {
          if( itsOwn ) 
            delete ptr;
        }  

      bool operator==( const SelfDestructPtr &x ) const {
        return *(ptr) == *(x.ptr);
      }

      bool operator!=( const SelfDestructPtr &x ) const {
        return *(ptr) != *(x.ptr);
      }
      
      T& operator*()  const            {return *ptr;}
      T* operator->() const            {return ptr;}
      T* get()        const            {return ptr;}
      T* release()    const  {itsOwn = false; return ptr;}
      
    };
  
  
  /**
   * Iterator that allows to get the list of available tags in a
   * TagContainer.
   */
  class TagIterator 
    {
    public:
      /**
       * Get next item on the list.
       *
       * @return name of the tag
       */
      virtual const char *getNext() = 0;
      /**
       * Returns true if there is still an item left on the list.
       */
      virtual bool hasNext() const = 0;
    };

  typedef SelfDestructPtr<TagIterator> TagIteratorPtr;
  
/**
 * TagContainer interface allows to read and modify tags. A tag is "name"="value" pair.
 */
  class TagContainer
    {
    public:
      /**
       * Get a string tag of the name tagName from the TagContainer.
       * @param tagName name of the tag to retrieve
       * @return tag value or NULL if tag was not found
       */
      virtual const char* getString( const char *tagName ) = 0;

      /**
       * Set or add a string tag of the name tagName. 
       * @param tagName name of the tag to add or set
       * @param tagValue value of the tag
       */      
      virtual void setString( const char *tagName, const char *tagValue ) = 0;
      
      /**
       * Removes (if exists) a tag of the name tagName from the TagContainer.
       * @param tagName name of the tag to remove
       */
      virtual void removeTag( const char *tagName ) = 0;

      /**
       * Use TagIterator to iterate over all tags in the TagContainer.
       * TagIteratorPtr is a smart pointer, which destructs
       * TagIterator when TagIteratorPtr is destructed. Use ->
       * operator to access TagIterator members from a TagIteratorPtr
       * object.
       *
       * To iterate over all tags, use the following code:
       * <code>
       * pfs::TagIteratorPtr it( frame->getTags()->getIterator() );
       * while( it->hasNext() ) {
       *   const char *tagName = it->getNext();
       *   //Do something
       * }
       * </code>
       */
      virtual TagIteratorPtr getIterator() const = 0;
    };


/**
 * Channel interface represents a 2D rectangular array with
 * associated tags.
 */
  class Channel : public Array2D {
  public:
    /**
     * Gets width of the channel (in pixels).
     * This is a synonym for Array2D::getCols().
     */
    int getWidth() const {
      return getCols();
    }

    /**
     * Gets height of the channel (in pixels).
     * This is a synonym for Array2D::getRows().
     */
    virtual int getHeight() const
      {
        return getRows();
      }

    /**
     * Gets name of the channel. 
     */
    virtual const char *getName() const = 0;
        
    /**
     * Returns TagContainer that can be used to access or modify
     * tags associated with this Channel object.
     */
    virtual TagContainer *getTags() = 0;

    /**
     * For performance reasons, the channels can be accessed as a
     * table of float values. Data is given in row-major order, i.e.
     * it is indexed data[x+y*width]. If performance is not crucial,
     * use Array2D interface instead.
     *
     * @return a table of floats of the size width*height
     */
    virtual float *getRawData() = 0;
  };

  /**
   * Iterator that allows to get the list of available channels in a frame.
   */
  class ChannelIterator 
    {
    public:
      /**
       * Get next item on the list.
       */
      virtual Channel *getNext() = 0;
      /**
       * Returns true if there is still an item left on the list.
       */
      virtual bool hasNext() const = 0;
    };
  
  typedef SelfDestructPtr<ChannelIterator> ChannelIteratorPtr;
  
/**
 * Interface representing a single PFS frame. Frame may contain 0
 * or more channels (e.g. color XYZ, depth channel, alpha
 * channnel). All the channels are of the same size. Frame can
 * also contain additional information in tags (see getTags).
 */
  class Frame {
  public:
    /**
     * Gets width of the channels (in pixels).
     */
    virtual int getWidth() const = 0;

    /**
     * Gets height of the channels (in pixels).
     */
    virtual int getHeight() const = 0;

    /**
     * Gets color channels in XYZ color space. May return NULLs
     * if such channels do not exist. Values assigned to
     * X, Y, Z are always either all NULLs or valid pointers to
     * channels.
     *
     * @param X [out] a pointer to store X channel in
     * @param Y [out] a pointer to store Y channel in
     * @param Z [out] a pointer to store Z channel in
     */
    virtual void getXYZChannels( Channel* &X, Channel* &Y, Channel* &Z ) = 0;

    /**
     * Creates color channels in XYZ color space. If such channels
     * already exists, returns existing channels, rather than
     * creating new ones.  Note, that nothing can be assumed about
     * the content of each channel.
     *
     * @param X [out] a pointer to store X channel in
     * @param Y [out] a pointer to store Y channel in
     * @param Z [out] a pointer to store Z channel in
     */
    virtual void createXYZChannels( Channel* &X, Channel* &Y, Channel* &Z ) = 0;

    /**
     * Gets a named channel. 
     *
     * @param name [in] name of the channel. Name must be 8 or less
     * character long.
     * @return channel or NULL if the channel does not exist
     */        
    virtual Channel* getChannel( const char *name ) = 0;

    /**
     * Creates a named channel. If the channel already exists, returns
     * existing channel.
     *
     * Note that new channels should be created only for the first
     * frame. The channels should not changes for the subsequent
     * frames of a sequence.
     *
     * @param name [in] name of the channel. Name must be 8 or less
     * character long.
     * @return existing or newly created channel
     */        
    virtual Channel* createChannel( const char *name ) = 0;


    /**
     * Removes a channel. It is safe to remove the channel pointed by
     * the ChannelIterator.
     *
     * @param channel [in] channel that should be removed.
     */        
    virtual void removeChannel( Channel *channel ) = 0;
    
    /**
     * DEPRECIATED!!! Use getIterator instead.
     *
     * Returns iterator for all available channels.
     *
     * Note that only one iterator for particular frame can be used at
     * a time. This method returns each time the same data structure,
     * so the iterator from previous call is lost after the call. The
     * iterator MUST NOT be deleted after use.
     *
     * Object ChannelIterator MUST NOT be freed. It's responsibility
     * of a Frame object.
     */
    virtual ChannelIterator *getChannels() = 0;
    
    /**
     * Use ChannelIterator to iterate over all Channels in the Frame.
     * ChannelIteratorPtr is a smart pointer, which destructs
     * ChannelIterator when ChannelIteratorPtr is destructed. Use ->
     * operator to access ChannelIterator members from a
     * ChannelIteratorPtr object.
     *
     * To iterate over all channels, use the following code:
     * <code>
     * pfs::ChannelIteratorPtr it( frame->getChannelIterator() );
     * while( it->hasNext() ) {
     *   pfs::Channel *ch = cit->getNext();
     *   //Do whatever is needed
     * }
     * </code>
     */
    virtual ChannelIteratorPtr getChannelIterator() = 0;
    
    /**
     * Returns TagContainer that can be used to access or modify
     * tags associated with this Frame object.
     */
    virtual TagContainer *getTags() = 0;

    virtual ~Frame() = 0;
    
  };

  /**
   * Copy all tags from both the frame and its channels to the
   * destination frame. If there is no corresponding destination
   * channel for a source channel, the tags from that source channel
   * will not be copied. Note, that all tags in the destination
   * channel will be removed before copying. Therefore after this
   * operation, the destination will contain exactly the same tags as
   * the source.
   */
  void copyTags( Frame *from, Frame *to );
  
  /**
   * Copy all tags from one container into another. Note, that all
   * tags in the destination channel will be removed before
   * copying. Therefore after this operation, the destination will
   * contain exactly the same tags as the source.
   */
  void copyTags( const TagContainer *from, TagContainer *to );

  
  class DOMIOImpl;

/**
 * Reading and writing frames in PFS format from/to streams.
 */
  class DOMIO {
    DOMIOImpl *impl;
  public:
    DOMIO();
    ~DOMIO();

    /**
     * Creates a frame that can be latter written to the stream
     * using writeFrame method. This method and readFrame are the
     * only way to create Frame objects.
     *
     * Note: Frame object must be released with freeFrame methods
     * as soon as it is no longer needed. Otherwise the
     * application will run out of memory.
     *
     * @param width width of the frame to create
     * @param height height of the frame to create
     * @return Frame object that can be modified and written back to PFS
     * stream using writeFrame method
     */
    Frame *createFrame( int width, int height );

    /**
     * Read PFS frame from the input Stream. This method and
     * createFrame are the only way to create Frame objects.
     *
     * Note: Frame object must be released with freeFrame methods
     * as soon as it is no longer needed. Otherwise the
     * application will run out of memory.
     *
     * @param inputStream read frame from that stream
     * @return Frame object that contains PFS frame read from
     * the stream. NULL if there are no more frames.
     */
    Frame *readFrame( FILE *inputStream );

    /**
     * Writes Frame object to outputStream in PFS format.
     *
     * @param frame Frame object to be written. This object
     * must be created with readFrame or createFrame method.
     * @param outputStream write frame to that stream
     */
    void writeFrame( Frame *frame, FILE *outputStream );

    /**
     * Deallocated memory allocated with createFrame or readFrame. Must
     * be called as soon as frame is not needed. Pointer to a frame is
     * invalid after this method call.
     *
     * @param frame Frame object to be freed
     */
    void freeFrame( Frame *frame );
  };



  /**
   * A pair of a file name and file handler, returned from
   * FrameFileIterator.
   */
  struct FrameFile
  {
    FrameFile( FILE *fh, const char* fileName ): fh(fh), fileName( fileName )
    {
    }

    /**
     * File handler.
     */
    FILE *fh;

    /**
     * File name.
     */
    const char *fileName;
  };


  class FrameFileIteratorImpl;

/**
 * Utility class that can be used to iterate over file names
 * specified as command line arguments. It can handle patterns,
 * like frame%04d.hdr, where %04d is replaced with specified
 * range of frame numbers.
 *
 */
  class FrameFileIterator
    {
      FrameFileIteratorImpl *impl;
    public:
      /**
       * Creates new iterator over frame files. Command line
       * arguments are parsed and all recognized arguments are
       * removed.
       *
       * @param argc argument count passed to program's main function.
       * @param argv argument values passed to program's main function.
       * @param fopenMode mode used to fopen frame files, usually "rb" or "wb"
       * @param fileNamePrefix each frame pattern must be preceded
       * with this string (for example "-i'). If NULL, every argument that
       * does not start with "-" is treated as a frame pattern.
       * @param stdinout if set, treat '-' file name specially and instead
       * of opening a named file, use filedescriptor passed as this parameter.
       * It should be used to get or write data to stdin / stdout.
       * @param optstring parameter string passed to getopt()
       * function. When optstring != NULL, FrameFileIterator will skip
       * all parameters and their required arguments. Optional
       * arguments are not handled.
       * @param getopt_long parameter structure passed to getopt_long()
       * function. When getopt_long != NULL, FrameFileIterator will skip
       * all parameters and their required arguments. Optional
       * arguments are not handled.
       * @throws CommandLineException on bad syntax of command line options
       */
      FrameFileIterator( int &argc, char* argv[], const char *fopenMode,
        const char *fileNamePrefix = NULL, FILE *stdinout = NULL,
        const char *optstring = NULL, const struct option *getopt_long = NULL );
      ~FrameFileIterator();

      /**
       * Get the file handle FILE* and file name for the next
       * frame. Note that fileName string is valid until next
       * call to getNextFrameFile or closeFrameFile.
       *
       * When file handle is no longer needed, closeFileFile
       * should be called.
       *
       * @return file handle FILE* and file name of the next frame.
       * Returns file handle == NULL if there are no more frames.
       *
       * @throws Exception if the file is not found
       */
      FrameFile getNextFrameFile( );

      /**
       * Close file openned with getNextFrameFile.
       *
       * @param frameFile FrameFile object returned from getNextFrameFile
       */
      void closeFrameFile( FrameFile &frameFile );

      static void printUsage( FILE *out, const char *progName );

    };


/// This enum is used to specify color spaces for transformColorSpace function
  enum ColorSpace
    {
      CS_XYZ = 0,         ///< Absolute XYZ space, reference white - D65, Y is calibrated luminance in cd/m^2
      CS_RGB,             ///< Absolute XYZ space, reference white - D65
      CS_SRGB,            ///< sRGB color space for LDR images (see
                          ///www.srgb.com). The possible pixel values
                          ///for R, G and B channel should be within
                          ///range 0-1 (the values above or below this
                          ///range will be clamped). Peak luminance
                          ///level of the display is 80cd/m^2.
      CS_YUV,             ///< Perceptually uniform u and v color coordinates, Y is calibrated luminance in cd/m^2
      CS_Yxy,              ///< Luminance and normalized chromacities (x=X/(X+Y+Z), y=Y/(X+Y+Z))
      CS_LAST             ///< For internal purposes only
    };

/**
 * Transform color channels from one color space into
 * another. Input and output channels may point to the same data
 * for in-memory transform.
 *
 * @param inCS input color space
 * @param inC1 first color channel of the input image
 * @param inC2 second color channel of the input image
 * @param inC3 third color channel of the input image
 * @param outCS output color space
 * @param outC1 first color channel of the output image
 * @param outC2 second color channel of the output image
 * @param outC3 third color channel of the output image
 */
  void transformColorSpace( ColorSpace inCS,
    const Array2D *inC1, const Array2D *inC2, const Array2D *inC3,
    ColorSpace outCS,
    Array2D *outC1, Array2D *outC2, Array2D *outC3 );


/**
 * General exception class used to throw exceptions from pfs library.
 */
  class Exception
    {
      char msg[1024];
    public:
      /**
       * Creates a new exception.
       *
       * @param message description of the cause for the
       * exception. The copy of the message string is made, so it can
       * be freed after creating Exception.
       */
      Exception( const char* const message )
	{
          strcpy( msg, message );
	}
			
      ~Exception() {};

      /**
       * Returns the description of the problem.
       *
       * @return text description of the cause for the exception
       */
      const char* getMessage()
        {
          return msg;
	}
    };


/**
 * Exception class used to throw exceptions from FileFileIterator class.
 */
  class CommandLineException: public Exception
    {
    public:
      CommandLineException( const char* const message ): Exception( message )
	{
	}
    };


}



#endif

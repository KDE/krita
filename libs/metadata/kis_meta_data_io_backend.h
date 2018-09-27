/*
*  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
*
*  This library is free software; you can redistribute it and/or modify
*  it under the terms of the GNU Lesser General Public License as published by
*  the Free Software Foundation; either version 2.1 of the License, or
*  (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef _KIS_META_DATA_IO_BACKEND_H_
#define _KIS_META_DATA_IO_BACKEND_H_

#include <kritametadata_export.h>

#include <KoGenericRegistry.h>

class QIODevice;

namespace KisMetaData
{
class Store;
/**
 * This is a the interface for input or output backend to KisMetaData.
 * For instance, to add support to exif or xmp or iptc or dublin core
 * or anything else, it is needed to extend this interface.
 */
class KRITAMETADATA_EXPORT IOBackend
{
public:

    /**
     * Tell whether the backend input/output from/to binary data
     * or text (XML or RDF) data.
     */
    enum BackendType {
        Binary,
        Text
    };

    enum HeaderType {
        NoHeader, ///< Don't append any header
        JpegHeader ///< Append Jpeg-style header
    };

public:

    virtual ~IOBackend() {};

    virtual QString id() const = 0;

    virtual QString name() const = 0;

    /**
     * @return the type of the backend
     */
    virtual BackendType type() const = 0;

    /**
     * @return tell if this backend support saving
     */
    virtual bool supportSaving() const = 0;

    /**
     * @param store the list of metadata to save
     * @param ioDevice the device to where the metadata will be saved
     * @param headerType determine if an header must be prepend to the binary header, and if it does,
     *                   which type of header
     * @return true if the save was successful (XXX: actually, all backends always return true...)
     */
    virtual bool saveTo(Store* store, QIODevice* ioDevice, HeaderType headerType = NoHeader) const = 0;

    /**
     * @param store the list of metadata
     * @return true if this backend is capable of saving all the metadata
     * of the store
     */
    virtual bool canSaveAllEntries(Store* store) const = 0;

    /**
     * @return true if this backend support loading
     */
    virtual bool supportLoading() const = 0;

    /**
     * @param store the list of metadata to load
     * @param ioDevice the device from where the metadata will be loaded
     * @return true if the load was successful
     */
    virtual bool loadFrom(Store* store, QIODevice* ioDevice) const = 0;
};

class KRITAMETADATA_EXPORT IOBackendRegistry : public KoGenericRegistry<IOBackend*>
{

public:

    IOBackendRegistry();
    ~IOBackendRegistry() override;
    static IOBackendRegistry* instance();

};

}


#endif

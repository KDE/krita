
/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KROSS_KRITACOREKRS_ITERATOR_H
#define KROSS_KRITACOREKRS_ITERATOR_H

#include <QObject>
#include <QList>
#include <QListIterator>
#include <klocale.h>

#include <KoColorTransformation.h>

#include "krs_paint_device.h"

#include <kis_paint_device.h>
#include <kis_types.h>

//#include "../scriptingmonitor.h"

namespace Scripting {

/**
 * This object allow to read the value of pixel one by one.
 */
class ConstIteratorBase : public QObject
{
        Q_OBJECT
    public:
        ConstIteratorBase(QObject* parent) : QObject(parent) {
            setObjectName("KritaIterator");
            // Connect the Monitor to know when the invalidating of iterator is needed
            //connect(ScriptingMonitor::instance(), SIGNAL(executionFinished(const Kross::Api::ScriptAction* )), this, SLOT(invalidate()));
        }
        virtual ~ConstIteratorBase() {}

    public slots:

        /**
         * Increment the position, and go to the next pixel. The
         * returned value is true if the iterator reached the
         * end (so, it's like calling next() and after that asking
         * what isDone() returns).
         */
        virtual bool next() = 0;

        /**
         * Return true if the iterator is at the end, what means, that
         * there are no more pixels available.
         */
        virtual bool isDone() = 0;

        /**
         * Return the current column the iterator is on. The value will
         * be always smaller then the width of the image/layer.
         */
        virtual int x() = 0;

        /**
         * Return the current row the iterator is on. The value will
         * be always smaller then the height of the image/layer.
         */
        virtual int y() = 0;

        /**
         * Return the value the current pixel has in the channel
         * number \p channelnr . If the channelnr is out of range
         * (as in >= what \p channelCount() returns) a invalid
         * QVariant is returned.
         */
        virtual QVariant channel(uint channelnr) = 0;

        /**
         * Return the number of channels the current pixmap has. As
         * example for RGBA-images 4 is returned cause R is one channel,
         * while G, B and A are other channels what makes 4 total.
         */
        virtual uint channelCount() = 0;

        /**
         * Return the current pixel. The pixel itself may have depending
         * on the used colorspace n colors where each color has it's own
         * channel. So, as example if the colorspace is RGBA, the returned
         * list has exact 4 items. The first one is a value 0-255 for the
         * color red, the second for green and the theird for blue while
         * the forth is the alpha-channel.
         */
        virtual QVariantList pixel() = 0;

    private slots:
        virtual void invalidateIterator() = 0;
};

/**
 * \internal template class that implements \a IteratorBase to provide
 * iterators for walking over the pixels of an image or a layer.
 */
template<class _T_It>
class ConstIterator : public ConstIteratorBase
{
    public:
        ConstIterator(ConstPaintDevice* layer, _T_It it)
            : ConstIteratorBase(layer)
            , m_it(new _T_It(it))
            , m_layer(layer->paintDevice())
        {
        }

        virtual ~ConstIterator()
        {
            invalidateIterator();
        }

    private:

        bool isDone()
        {
            return m_it->isDone();
        }

        bool next()
        {
            ++(*m_it);
            return m_it->isDone();
        }

        int x()
        {
            return m_it->x();
        }

        int y()
        {
            return m_it->y();
        }

        QVariant channel(uint channelnr)
        {
            QList<KoChannelInfo*> channels = m_layer->colorSpace()->channels();
            return channelnr < uint(channels.count()) ? channelValue(channels[channelnr]) : QVariant();
        }

        uint channelCount()
        {
            return m_layer->colorSpace()->channels().count();
        }

        QVariantList pixel()
        {
            QVariantList pixel;
            QList<KoChannelInfo*> channels = m_layer->colorSpace()->channels();
            for(QList<KoChannelInfo*>::iterator itC = channels.begin(); itC != channels.end(); ++itC)
                pixel.push_back( channelValue(*itC) );
            return pixel;
        }


    private:

        virtual void invalidateIterator()
        {
            delete m_it; m_it = 0;
        }

        inline QVariant channelValue(KoChannelInfo* ci)
        {
            quint8* data = (quint8*)(m_it->rawData() + ci->pos());
            switch(ci->channelValueType()) {
                case KoChannelInfo::UINT8:
                    return *data;
                case KoChannelInfo::UINT16:
                    return *((quint16*) data);
                case KoChannelInfo::FLOAT32:
                    return *((float*) data);
                default:
                    kDebug(41011) << "Unsupported data format in script" << endl;
                    return QVariant();
            }
        }

    private:
        _T_It* m_it;
        const KisPaintDeviceSP m_layer;
};

}

#endif

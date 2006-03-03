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

#include <qobject.h>
#include <api/class.h>

#include <klocale.h>

#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_types.h>

#include <kis_script_monitor.h>

namespace Kross {

namespace KritaCore {

//<beurk> stupid Qt which doesn't support templated QObject
class IteratorMemoryManaged {
    public:
        virtual void invalidateIterator() = 0;
};
    
class IteratorMemoryManager : public QObject {
    Q_OBJECT
    public:
        IteratorMemoryManager(IteratorMemoryManaged* it) : m_it(it)
        {
            // Connect the Monitor to know when the invalidating of iterator is needed
            connect(KisScriptMonitor::instance(), SIGNAL(executionFinished(const Kross::Api::ScriptAction* )), this, SLOT(invalidateIterator()));

        }
    public slots:
        void invalidateIterator()
        {
            m_it->invalidateIterator();
        }
    private:
        IteratorMemoryManaged* m_it;
};
//</beurk>
/**
 * This object allow to change the value of pixel one by one.
 * The name of some function depends of the colorspace, for instance, if
 * the colorspace of the layer is RGB, you will have setR, setG, setB... and for CMYK,
 * setC, setM, setY, setK. In the doc bellow we will consider the colorspace is called ABC with
 * three channels : A, B and C.
 * 
 * Function: setA setB setC
 * Those functions take one argument:
 *  - the new value of one of the channel of this pixel
 * 
 * Function: setABC
 * Set the value of all channels.
 * This function take one argument:
 *  - an array with the new value for all channels
 */
template<class _T_It>
class Iterator : public Kross::Api::Class<Iterator<_T_It> >, private IteratorMemoryManaged
{
    public:
        Iterator(_T_It it, KisPaintLayerSP layer) : Kross::Api::Class<Iterator<_T_It> >("KritaIterator"), m_itmm (new IteratorMemoryManager(this)), m_it(new _T_It(it)), nchannels(layer->paintDevice()->nChannels()), m_layer(layer)
        {
            // navigate in the iterator
            this->addFunction("next",
                new Kross::Api::ConstFunction0< Iterator<_T_It> >(
                    this, &Iterator<_T_It>::next ) );
            this->addFunction("isDone",
                new Kross::Api::ConstFunction0< Iterator<_T_It> >(
                    this, &Iterator<_T_It>::isDone ) );
    
            // get/set value
            QValueVector<KisChannelInfo *> channels = layer->paintDevice()->colorSpace()->channels();
            QString initiales = "";
            for(QValueVector<KisChannelInfo *>::iterator itC = channels.begin(); itC != channels.end(); itC++)
            {
                KisChannelInfo * ci = *itC;
                initiales += ci->name().left(1);
                switch(ci->channelValueType())
                {
                    case KisChannelInfo::UINT8:
                        this->addFunction("get"+ci->name(),
                            new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                                this, &Iterator<_T_It>::getChannelUINT8, ci->pos() ) );
                        this->addFunction("set"+ci->name(),
                            new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                                this, &Iterator<_T_It>::setChannelUINT8, ci->pos() ) );
                        break;
                    case KisChannelInfo::UINT16:
                        this->addFunction("get"+ci->name(),
                            new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                                this, &Iterator<_T_It>::getChannelUINT16, ci->pos() ) );
                        this->addFunction("set"+ci->name(),
                            new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                                this, &Iterator<_T_It>::setChannelUINT16, ci->pos() ) );
                        break;
                    case KisChannelInfo::FLOAT32:
                        this->addFunction("get"+ci->name(),
                            new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                                this, &Iterator<_T_It>::getChannelFLOAT, ci->pos() ) );
                        this->addFunction("set"+ci->name(),
                            new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                                this, &Iterator<_T_It>::setChannelFLOAT, ci->pos() ) );
                        break;
                    default:
                        kdDebug(41011) << "unsupported data format in scripts" << endl;
                        break;
                }
            }
            initiales = initiales.upper();
            // set/get general
            addFunction("set" + initiales, &Iterator::setPixel, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::List") );
            addFunction("get" + initiales, &Iterator::getPixel);
            kdDebug(41011) << ( "get" + initiales ) << endl;
            // Various colorSpace
            addFunction("invertColor", &Iterator::invertColor);
            addFunction("darken", &Iterator::darken);
        }
    
        ~Iterator()
        {
            invalidateIterator();
            delete m_itmm;
        }
        virtual const QString getClassName() const {
            return "Kross::KritaCore::KrsDoc";
        };
    private:
        /**
         * Darken a pixel.
         * This functions at least one argument:
         *  - shade amount use to darken all color channels
         * 
         * This function can take the following optional argument:
         *  - compensation to limit the darkening
         */
        Kross::Api::Object::Ptr darken(Kross::Api::List::Ptr args)
        {
            Q_INT32 shade = Kross::Api::Variant::toUInt( args->item(0) );
            bool compensate = (args->count() == 2);
            double compensation = compensate ? Kross::Api::Variant::toDouble( args->item(2) ) : 0.;
            m_layer->paintDevice()->colorSpace()->darken(m_it->rawData(), m_it->rawData(), shade, compensate, compensation, 1);
            return 0;
        }
        /**
         * Invert the color of a pixel.
         */
        Kross::Api::Object::Ptr invertColor(Kross::Api::List::Ptr )
        {
            m_layer->paintDevice()->colorSpace()->invertColor(m_it->rawData(), 1);
            return 0;
        }
        /**
         * Increment the positon, and go to the next pixel.
         */
        Kross::Api::Object::Ptr next()
        {
            ++(*m_it);
            return new Kross::Api::Variant(m_it->isDone());
        }
        /**
         * Return true if the iterator is at the end, and that no more pixels are available.
         */
        Kross::Api::Object::Ptr isDone()
        {
            return new Kross::Api::Variant(m_it->isDone());
        }
        Kross::Api::Object::Ptr getChannelUINT8(Kross::Api::List::Ptr, uint channelpos)
        {
            Q_UINT8* data = (Q_UINT8*)(m_it->rawData() + channelpos);
            return new Kross::Api::Variant( * data);
        }
        Kross::Api::Object::Ptr setChannelUINT8(Kross::Api::List::Ptr args, uint channelpos)
        {
            Q_UINT8* data = (Q_UINT8*)(m_it->rawData() + channelpos); //*(uint*)channelpos);
            *data = Kross::Api::Variant::toUInt( args->item(0) );
            return 0;
        }
        Kross::Api::Object::Ptr getChannelUINT16(Kross::Api::List::Ptr, uint channelpos)
        {
            Q_UINT16* data = (Q_UINT16*)(m_it->rawData() + channelpos);
            return new Kross::Api::Variant( * data);
        }
        Kross::Api::Object::Ptr setChannelUINT16(Kross::Api::List::Ptr args, uint channelpos)
        {
            Q_UINT16* data = (Q_UINT16*)(m_it->rawData() + channelpos);
            *data =  Kross::Api::Variant::toUInt( args->item(0) );
            return 0;
        }
        Kross::Api::Object::Ptr getChannelFLOAT(Kross::Api::List::Ptr, uint channelpos)
        {
            float* data = (float*)(m_it->rawData() + channelpos);
            return new Kross::Api::Variant( * data);
        }
        Kross::Api::Object::Ptr setChannelFLOAT(Kross::Api::List::Ptr args, uint channelpos)
        {
            float* data = (float*)(m_it->rawData() + channelpos);
            *data = Kross::Api::Variant::toUInt( args->item(0) );
            return 0;
        }
        Kross::Api::Object::Ptr getPixel(Kross::Api::List::Ptr)
        {
            QValueVector<KisChannelInfo *> channels = m_layer->paintDevice()->colorSpace()->channels();
            QValueList<QVariant> pixel;
            for(QValueVector<KisChannelInfo *>::iterator itC = channels.begin(); itC != channels.end(); itC++)
            {
                KisChannelInfo * ci = *itC;
                Q_UINT8* data = (Q_UINT8*)(m_it->rawData() + ci->pos());
                switch(ci->channelValueType())
                {
                    case KisChannelInfo::UINT8:
                        pixel.push_back( *data);
                        break;
                    case KisChannelInfo::UINT16:
                        pixel.push_back( *((Q_UINT16*) data) );
                        break;
                    case KisChannelInfo::FLOAT32:
                        pixel.push_back( *((float*) data) );
                        break;
                    default:
                        kdDebug(41011) << i18n("An error has occurred in %1").arg("getPixel") << endl;
                        kdDebug(41011) << i18n("unsupported data format in scripts") << endl;
                        break;
                }
            }
            return new Kross::Api::Variant( pixel);
        }
        Kross::Api::Object::Ptr setPixel(Kross::Api::List::Ptr args)
        {
            QValueList<QVariant> pixel = Kross::Api::Variant::toList( args->item(0) );
            QValueVector<KisChannelInfo *> channels = m_layer->paintDevice()->colorSpace()->channels();
            uint i = 0;
            for(QValueVector<KisChannelInfo *>::iterator itC = channels.begin(); itC != channels.end(); itC++, i++)
            {
                KisChannelInfo * ci = *itC;
                Q_UINT8* data = (Q_UINT8*)(m_it->rawData() + ci->pos());
                switch(ci->channelValueType())
                {
                    case KisChannelInfo::UINT8:
                        *data = pixel[i].toUInt();
                        break;
                    case KisChannelInfo::UINT16:
                        *((Q_UINT16*) data) = pixel[i].toUInt();
                        break;
                    case KisChannelInfo::FLOAT32:
                        *((float*) data) = pixel[i].toDouble();
                        break;
                    default:
                        kdDebug(41011) << i18n("An error has occurred in %1").arg("setPixel") << endl;
                        kdDebug(41011) << i18n("unsupported data format in scripts") << endl;
                        break;
                }
            }
            return 0;
        }
    private:
        virtual void invalidateIterator()
        {
            kdDebug(41011) << "invalidating iterator" << endl;
            if(m_it)
            {
                kdDebug(41011) << "deleting iterator" << endl;
                delete m_it;
            }
            m_it = 0;
            kdDebug() << " Iterator = " << m_it << endl;
        }
    private:
        IteratorMemoryManager* m_itmm;
        _T_It* m_it;
        int nchannels;
        KisPaintLayerSP m_layer;
};

}

}

#endif

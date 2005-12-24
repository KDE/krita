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

#include <api/class.h>
#include <api/event.h>

#include <kis_types.h>

namespace Kross {

namespace KritaCore {

/**
@author Cyrille Berger
*/
template<class _T_It>
class Iterator : public Kross::Api::Class<Iterator<_T_It> >
{
    public:
    Iterator(_T_It it, KisLayerSP layer) : Kross::Api::Class<Iterator<_T_It> >("KritaIterator"), m_it(it), nchannels(layer->nChannels())
    {
        this->addFunction("next",
            new Kross::Api::ConstFunction0< Iterator<_T_It> >(
                this, &Iterator<_T_It>::next ) );
        this->addFunction("isDone",
            new Kross::Api::ConstFunction0< Iterator<_T_It> >(
                this, &Iterator<_T_It>::isDone ) );

        QValueVector<KisChannelInfo *> channels = layer->colorSpace()->channels();
        kdDebug() << layer->colorSpace()->id().name() << endl;
        for(QValueVector<KisChannelInfo *>::iterator itC = channels.begin(); itC != channels.end(); itC++)
        {
            KisChannelInfo * ci = *itC;
            kdDebug() << ci->pos() << " " << ci->channelValueType() << " " << Kross::Api::Variant::toUInt(new Api::Variant( ci->pos())) << endl;
            switch(ci->channelValueType())
            {
                case KisChannelInfo::UINT8:
                    kdDebug() << "UINT8 channel" << endl;

                    this->addFunction("get"+ci->name(),
                        new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                            this, &Iterator<_T_It>::getChannelUINT8, ci->pos() ) );
                    this->addFunction("set"+ci->name(),
                        new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                            this, &Iterator<_T_It>::setChannelUINT8, ci->pos() ) );
                    break;
                case KisChannelInfo::UINT16:
                    kdDebug() << "UINT16 channel" << endl;

                    this->addFunction("get"+ci->name(),
                        new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                            this, &Iterator<_T_It>::getChannelUINT16, ci->pos() ) );
                    this->addFunction("set"+ci->name(),
                        new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                            this, &Iterator<_T_It>::setChannelUINT16, ci->pos() ) );
                    break;
                case KisChannelInfo::FLOAT32:
                    kdDebug() << "FLOAT32 channel" << endl;

                    this->addFunction("get"+ci->name(),
                        new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                            this, &Iterator<_T_It>::getChannelFLOAT, ci->pos() ) );
                    this->addFunction("set"+ci->name(),
                        new Kross::Api::VarFunction1< Iterator<_T_It> , uint >(
                            this, &Iterator<_T_It>::setChannelFLOAT, ci->pos() ) );
                    break;
                default:
                    kdDebug() << "unsupported data format in scripts" << endl;
                    break;
            }
        }
    }

    ~Iterator()
    {
    }
    virtual const QString getClassName() const {
        return "Kross::KritaCore::KrsDoc";
    };
    private:
        Kross::Api::Object::Ptr next()
        {
            ++m_it;
            return new Kross::Api::Variant(m_it.isDone());
        }
        Kross::Api::Object::Ptr isDone()
        {
            return new Kross::Api::Variant(m_it.isDone());
        }
        Kross::Api::Object::Ptr getChannelUINT8(Kross::Api::List::Ptr, uint channelpos)
        {
            Q_UINT8* data = (Q_UINT8*)(m_it.rawData() + channelpos);
            return new Kross::Api::Variant( * data);
        }
        Kross::Api::Object::Ptr setChannelUINT8(Kross::Api::List::Ptr args, uint channelpos)
        {
            Q_UINT8* data = (Q_UINT8*)(m_it.rawData() + channelpos); //*(uint*)channelpos);
            *data = Kross::Api::Variant::toUInt( args->item(0) );
            return 0;
        }
        Kross::Api::Object::Ptr getChannelUINT16(Kross::Api::List::Ptr, uint channelpos)
        {
            Q_UINT16* data = (Q_UINT16*)(m_it.rawData() + channelpos);
            return new Kross::Api::Variant( * data);
        }
        Kross::Api::Object::Ptr setChannelUINT16(Kross::Api::List::Ptr args, uint channelpos)
        {
            Q_UINT16* data = (Q_UINT16*)(m_it.rawData() + channelpos);
            *data =  Kross::Api::Variant::toUInt( args->item(0) );
            return 0;
        }
        Kross::Api::Object::Ptr getChannelFLOAT(Kross::Api::List::Ptr, uint channelpos)
        {
            float* data = (float*)(m_it.rawData() + channelpos);
            return new Kross::Api::Variant( * data);
        }
        Kross::Api::Object::Ptr setChannelFLOAT(Kross::Api::List::Ptr args, uint channelpos)
        {
            float* data = (float*)(m_it.rawData() + channelpos);
            *data = Kross::Api::Variant::toUInt( args->item(0) );
            return 0;
        }
    private:
        _T_It m_it;
        int nchannels;
};

}

}

#endif

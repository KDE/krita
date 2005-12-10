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
        addFunction("next", &Iterator<_T_It>::next);
        addFunction("isDone", &Iterator<_T_It>::isDone);
        QValueVector<KisChannelInfo *> channels = layer->colorSpace()->channels();
        kdDebug() << layer->colorSpace()->id().name() << endl;
//         addFunction("getBlue", &Iterator<_T_It>::getChannelUINT8, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt", new Api::Variant(0),true) );
#if 1
        for(QValueVector<KisChannelInfo *>::iterator itC = channels.begin(); itC != channels.end(); itC++)
        {
            KisChannelInfo * ci = *itC;
            kdDebug() << ci->pos() << " " << ci->channelValueType() << " " << Kross::Api::Variant::toUInt(new Api::Variant( ci->pos())) << endl;
            switch(ci->channelValueType())
            {
                case KisChannelInfo::UINT8:
                    kdDebug() << "UINT8 channel" << endl;
                    addFunction("get"+ci->name(), &Iterator<_T_It>::getChannelUINT8, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt", new Api::Variant(ci->pos()),false) );
                    addFunction("set"+ci->name(), &Iterator<_T_It>::setChannelUINT8,  Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt", new Api::Variant( ci->pos()),false));
                    break;
                case KisChannelInfo::UINT16:
                    kdDebug() << "UINT16 channel" << endl;
                    addFunction("get"+ci->name(), &Iterator<_T_It>::getChannelUINT16,  Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt", new Api::Variant(ci->pos()),false));
                    addFunction("set"+ci->name(), &Iterator<_T_It>::setChannelUINT16,  Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt", new Api::Variant(ci->pos()),true));
                    break;
                case KisChannelInfo::FLOAT32:
                    kdDebug() << "FLOAT32 channel" << endl;
                    addFunction("get"+ci->name(), &Iterator<_T_It>::getChannelFLOAT,  Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::Double", new Api::Variant(ci->pos()),false));
                    addFunction("set"+ci->name(), &Iterator<_T_It>::setChannelFLOAT,  Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::Double") << Kross::Api::Argument("Kross::Api::Variant::UInt", new Api::Variant(ci->pos()),true));
                    break;
                default:
                    kdDebug() << "unsupported data format in scripts" << endl;
                    break;
            }
        }
#endif
    }

    ~Iterator()
    {
    }
    virtual const QString getClassName() const {
        return "Kross::KritaCore::KrsDoc";
    };
    private:
        Kross::Api::Object::Ptr next(Kross::Api::List::Ptr)
        {
            ++m_it;
            return new Kross::Api::Variant(m_it.isDone());
        }
        Kross::Api::Object::Ptr isDone(Kross::Api::List::Ptr)
        {
            return new Kross::Api::Variant(m_it.isDone());
        }
        Kross::Api::Object::Ptr getChannelUINT8(Kross::Api::List::Ptr args)
        {
            uint channelpos = Kross::Api::Variant::toUInt(args->item(0));
            Q_UINT8* data = (Q_UINT8*)(m_it.rawData() + channelpos);
            return new Kross::Api::Variant( * data);
        }
        Kross::Api::Object::Ptr setChannelUINT8(Kross::Api::List::Ptr args)
        {
            uint channelpos = Kross::Api::Variant::toUInt(args->item(1));
            Q_UINT8* data = (Q_UINT8*)(m_it.rawData() + channelpos);
            *data = Kross::Api::Variant::toUInt( args->item(0) );
            return 0;
        }
        Kross::Api::Object::Ptr getChannelUINT16(Kross::Api::List::Ptr args)
        {
            uint channelpos = Kross::Api::Variant::toUInt(args->item(0));
            Q_UINT16* data = (Q_UINT16*)(m_it.rawData() + channelpos);
            return new Kross::Api::Variant( * data);
        }
        Kross::Api::Object::Ptr setChannelUINT16(Kross::Api::List::Ptr args)
        {
            uint channelpos = Kross::Api::Variant::toUInt(args->item(1));
            Q_UINT16* data = (Q_UINT16*)(m_it.rawData() + channelpos);
            *data =  Kross::Api::Variant::toUInt( args->item(0) );
            return 0;
        }
        Kross::Api::Object::Ptr getChannelFLOAT(Kross::Api::List::Ptr args)
        {
            uint channelpos = Kross::Api::Variant::toUInt(args->item(0));
            float* data = (float*)(m_it.rawData() + channelpos);
            return new Kross::Api::Variant( * data);
        }
        Kross::Api::Object::Ptr setChannelFLOAT(Kross::Api::List::Ptr args)
        {
            uint channelpos = Kross::Api::Variant::toUInt(args->item(1));
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

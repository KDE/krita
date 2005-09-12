/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#include <kdebug.h>

#include "kis_global.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"

#define DEBUG_LAYERS 0

#if DEBUG_LAYERS
static int numLayers = 0;
#endif

namespace {

    class KisLayerCommand : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisLayerCommand(const QString& name, KisLayerSP layer);
        virtual ~KisLayerCommand() {}

        virtual void execute() = 0;
        virtual void unexecute() = 0;

    protected:
        void setUndo(bool undo);
        void notifyPropertyChanged();

        KisLayerSP m_layer;
    };

    KisLayerCommand::KisLayerCommand(const QString& name, KisLayerSP layer) :
        super(name), m_layer(layer)
    {
    }

    void KisLayerCommand::setUndo(bool undo)
    {
        if (m_layer -> undoAdapter()) {
            m_layer -> undoAdapter() -> setUndo(undo);
        }
    }

    void KisLayerCommand::notifyPropertyChanged()
    {
        if (m_layer -> image()) {
            m_layer -> image() -> notifyLayersChanged();
        }
    }

    class KisLayerLinkedCommand : public KisLayerCommand {
        typedef KisLayerCommand super;

    public:
        KisLayerLinkedCommand(KisLayerSP layer, bool oldLinked, bool newLinked);

        virtual void execute();
        virtual void unexecute();

    private:
        bool m_oldLinked;
        bool m_newLinked;
    };

    KisLayerLinkedCommand::KisLayerLinkedCommand(KisLayerSP layer, bool oldLinked, bool newLinked) :
        super(i18n("Link Layer"), layer)
    {
        m_oldLinked = oldLinked;
        m_newLinked = newLinked;
    }

    void KisLayerLinkedCommand::execute()
    {
        setUndo(false);
        m_layer -> setLinked(m_newLinked);
        notifyPropertyChanged();
        setUndo(true);
    }

    void KisLayerLinkedCommand::unexecute()
    {
        setUndo(false);
        m_layer -> setLinked(m_oldLinked);
        notifyPropertyChanged();
        setUndo(true);
    }

    class KisLayerLockedCommand : public KisLayerCommand {
        typedef KisLayerCommand super;

    public:
        KisLayerLockedCommand(KisLayerSP layer, bool oldLocked, bool newLocked);

        virtual void execute();
        virtual void unexecute();

    private:
        bool m_oldLocked;
        bool m_newLocked;
    };

    KisLayerLockedCommand::KisLayerLockedCommand(KisLayerSP layer, bool oldLocked, bool newLocked) :
        super(i18n("Lock Layer"), layer)
    {
        m_oldLocked = oldLocked;
        m_newLocked = newLocked;
    }

    void KisLayerLockedCommand::execute()
    {
        setUndo(false);
        m_layer -> setLocked(m_newLocked);
        notifyPropertyChanged();
        setUndo(true);
    }

    void KisLayerLockedCommand::unexecute()
    {
        setUndo(false);
        m_layer -> setLocked(m_oldLocked);
        notifyPropertyChanged();
        setUndo(true);
    }

    class KisLayerOpacityCommand : public KisLayerCommand {
        typedef KisLayerCommand super;

    public:
        KisLayerOpacityCommand(KisLayerSP layer, Q_UINT8 oldOpacity, Q_UINT8 newOpacity);

        virtual void execute();
        virtual void unexecute();

    private:
        Q_UINT8 m_oldOpacity;
        Q_UINT8 m_newOpacity;
    };

    KisLayerOpacityCommand::KisLayerOpacityCommand(KisLayerSP layer, Q_UINT8 oldOpacity, Q_UINT8 newOpacity) :
        super(i18n("Layer Opacity"), layer)
    {
        m_oldOpacity = oldOpacity;
        m_newOpacity = newOpacity;
    }

    void KisLayerOpacityCommand::execute()
    {
        setUndo(false);
        m_layer -> setOpacity(m_newOpacity);
        notifyPropertyChanged();
        setUndo(true);
    }

    void KisLayerOpacityCommand::unexecute()
    {
        setUndo(false);
        m_layer -> setOpacity(m_oldOpacity);
        notifyPropertyChanged();
        setUndo(true);
    }

}

KisLayer::KisLayer(KisColorSpace * colorSpace, const QString& name)
    : super(colorSpace, name),
      m_opacity(OPACITY_OPAQUE),
      m_linked(false),
      m_locked(false)
{
#if DEBUG_LAYERS
    numLayers++;
    kdDebug(DBG_AREA_CORE) << "LAYER " << name << " CREATED total now = " << numLayers << endl;
#endif
}

KisLayer::KisLayer(KisImage *img, const QString& name, Q_UINT8 opacity)
    : super(img, img -> colorSpace(), name),
      m_opacity(opacity),
      m_linked(false),
      m_locked(false)
{
#if DEBUG_LAYERS
    numLayers++;
    kdDebug(DBG_AREA_CORE) << "LAYER " << name << " CREATED total now = " << numLayers << endl;
#endif
}

KisLayer::KisLayer(KisImage *img, const QString& name, Q_UINT8 opacity, KisColorSpace * colorSpace)
    : super(img, colorSpace, name),
      m_opacity(opacity),
      m_linked(false),
      m_locked(false)
{
#if DEBUG_LAYERS
    numLayers++;
    kdDebug(DBG_AREA_CORE) << "LAYER " << name << " CREATED total now = " << numLayers << endl;
#endif
}

KisLayer::KisLayer(const KisLayer& rhs) : super(rhs)
{
#if DEBUG_LAYERS
    numLayers++;
    kdDebug(DBG_AREA_CORE) << "LAYER " << rhs.name() << " copy CREATED total now = " << numLayers << endl;
#endif
    if (this != &rhs) {
        m_opacity = rhs.m_opacity;
        //m_preserveTransparency = rhs.m_preserveTransparency;
        //m_initial = rhs.m_initial;
        m_linked = rhs.m_linked;
        m_locked = rhs.m_locked;
/*        if (rhs.m_mask)
            m_mask = new KisMask(*rhs.m_mask);*/
    }
}

KisLayer::~KisLayer()
{
#if DEBUG_LAYERS
    numLayers--;
    kdDebug(DBG_AREA_CORE) << "LAYER " << name() << " DESTROYED total now = " << numLayers << endl;
#endif
}

Q_UINT8 KisLayer::opacity() const
{
    return m_opacity;
}

void KisLayer::setOpacity(Q_UINT8 val)
{
    m_opacity = val;
}

KNamedCommand *KisLayer::setOpacityCommand(Q_UINT8 newOpacity)
{
    return new KisLayerOpacityCommand(this, opacity(), newOpacity);
}

bool KisLayer::linked() const
{
    return m_linked;
}

void KisLayer::setLinked(bool l)
{
    m_linked = l;
}

KNamedCommand *KisLayer::setLinkedCommand(bool newLinked)
{
    return new KisLayerLinkedCommand(this, linked(), newLinked);
}

const bool KisLayer::visible() const
{
    return super::visible() && m_opacity != OPACITY_TRANSPARENT;
}

void KisLayer::setVisible(bool v)
{
    super::setVisible(v);
}

bool KisLayer::locked() const
{
    return m_locked;
}

void KisLayer::setLocked(bool l)
{
    m_locked = l;
}

KNamedCommand *KisLayer::setLockedCommand(bool newLocked)
{
    return new KisLayerLockedCommand(this, locked(), newLocked);
}


#include "kis_layer.moc"

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
#ifndef KIS_LAYER_H_
#define KIS_LAYER_H_

#include "kis_paint_device_impl.h"
#include "kis_types.h"
#include <koffice_export.h>

class KNamedCommand;

class KRITACORE_EXPORT KisLayer : public KisPaintDeviceImpl {
    typedef KisPaintDeviceImpl super;

    Q_OBJECT

public:
    KisLayer(KisAbstractColorSpace * colorStrategy, const QString& name);
    KisLayer(KisImage *img, const QString& name, QUANTUM opacity);
    KisLayer(KisImage *img, const QString& name, QUANTUM opacity, KisAbstractColorSpace * colorStrategy);
    KisLayer(const KisLayer& rhs);
    virtual ~KisLayer();

public:

    // Called when the layer is made active
    virtual void activate() {};

    // Called when another layer is made active
    virtual void deactivate() {};

public:
    virtual const bool visible() const;
    virtual void setVisible(bool v);
    
    QUANTUM opacity() const;
    void setOpacity(QUANTUM val);
    KNamedCommand *setOpacityCommand(QUANTUM val);

    bool linked() const;
    void setLinked(bool l);
    KNamedCommand *setLinkedCommand(bool linked);

    bool locked() const;
    void setLocked(bool l);
    KNamedCommand *setLockedCommand(bool locked);


private:
    QUANTUM m_opacity;
    //bool m_preserveTransparency;
    //bool m_initial;
    bool m_linked;
    bool m_locked;
};

#endif // KIS_LAYER_H_


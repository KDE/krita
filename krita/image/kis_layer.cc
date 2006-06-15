/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#include <QIcon>
#include <QImage>

#include "kis_debug_areas.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"

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

        KisLayerSP m_layer;
    };

    KisLayerCommand::KisLayerCommand(const QString& name, KisLayerSP layer) :
        super(name), m_layer(layer)
    {
    }

    void KisLayerCommand::setUndo(bool undo)
    {
        if (m_layer->undoAdapter()) {
            m_layer->undoAdapter()->setUndo(undo);
        }
    }

    class KisLayerLockedCommand : public KisLayerCommand {
        typedef KisLayerCommand super;

    public:
        KisLayerLockedCommand(KisLayerSP layer, bool newLocked);

        virtual void execute();
        virtual void unexecute();

    private:
        bool m_newLocked;
    };

    KisLayerLockedCommand::KisLayerLockedCommand(KisLayerSP layer, bool newLocked) :
        super(i18n("Lock Layer"), layer)
    {
        m_newLocked = newLocked;
    }

    void KisLayerLockedCommand::execute()
    {
        setUndo(false);
        m_layer->setLocked(m_newLocked);
        setUndo(true);
    }

    void KisLayerLockedCommand::unexecute()
    {
        setUndo(false);
        m_layer->setLocked(!m_newLocked);
        setUndo(true);
    }

    class KisLayerOpacityCommand : public KisLayerCommand {
        typedef KisLayerCommand super;

    public:
        KisLayerOpacityCommand(KisLayerSP layer, quint8 oldOpacity, quint8 newOpacity);

        virtual void execute();
        virtual void unexecute();

    private:
        quint8 m_oldOpacity;
        quint8 m_newOpacity;
    };

    KisLayerOpacityCommand::KisLayerOpacityCommand(KisLayerSP layer, quint8 oldOpacity, quint8 newOpacity) :
        super(i18n("Layer Opacity"), layer)
    {
        m_oldOpacity = oldOpacity;
        m_newOpacity = newOpacity;
    }

    void KisLayerOpacityCommand::execute()
    {
        setUndo(false);
        m_layer->setOpacity(m_newOpacity);
        setUndo(true);
    }

    void KisLayerOpacityCommand::unexecute()
    {
        setUndo(false);
        m_layer->setOpacity(m_oldOpacity);
        setUndo(true);
    }

    class KisLayerVisibilityCommand : public KisLayerCommand {
        typedef KisLayerCommand super;

    public:
        KisLayerVisibilityCommand(KisLayerSP layer, bool newVisibility);

        virtual void execute();
        virtual void unexecute();

    private:
        bool m_newVisibility;
    };

    KisLayerVisibilityCommand::KisLayerVisibilityCommand(KisLayerSP layer, bool newVisibility) :
        super(i18n("Layer Visibility"), layer)
    {
        m_newVisibility = newVisibility;
    }

    void KisLayerVisibilityCommand::execute()
    {
        setUndo(false);
        m_layer->setVisible(m_newVisibility);
        setUndo(true);
    }

    void KisLayerVisibilityCommand::unexecute()
    {
        setUndo(false);
        m_layer->setVisible(!m_newVisibility);
        setUndo(true);
    }

    class KisLayerCompositeOpCommand : public KisLayerCommand {
        typedef KisLayerCommand super;

    public:
        KisLayerCompositeOpCommand(KisLayerSP layer, const KoCompositeOp& oldCompositeOp, const KoCompositeOp& newCompositeOp);

        virtual void execute();
        virtual void unexecute();

    private:
        KoCompositeOp m_oldCompositeOp;
        KoCompositeOp m_newCompositeOp;
    };

    KisLayerCompositeOpCommand::KisLayerCompositeOpCommand(KisLayerSP layer, const KoCompositeOp& oldCompositeOp,
                                       const KoCompositeOp& newCompositeOp) :
        super(i18n("Layer Composite Mode"), layer)
    {
        m_oldCompositeOp = oldCompositeOp;
        m_newCompositeOp = newCompositeOp;
    }

    void KisLayerCompositeOpCommand::execute()
    {
        setUndo(false);
        m_layer->setCompositeOp(m_newCompositeOp);
        setUndo(true);
    }

    void KisLayerCompositeOpCommand::unexecute()
    {
        setUndo(false);
        m_layer->setCompositeOp(m_oldCompositeOp);
        setUndo(true);
    }

    class KisLayerOffsetCommand : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisLayerOffsetCommand(KisLayerSP layer, const QPoint& oldpos, const QPoint& newpos);
        virtual ~KisLayerOffsetCommand();

        virtual void execute();
        virtual void unexecute();

    private:
        void moveTo(const QPoint& pos);

    private:
        KisLayerSP m_layer;
        QRect m_updateRect;
        QPoint m_oldPos;
        QPoint m_newPos;
    };

    KisLayerOffsetCommand::KisLayerOffsetCommand(KisLayerSP layer, const QPoint& oldpos, const QPoint& newpos) :
        super(i18n("Move Layer"))
    {
        m_layer = layer;
        m_oldPos = oldpos;
        m_newPos = newpos;

        QRect currentBounds = m_layer->exactBounds();
        QRect oldBounds = currentBounds;
        oldBounds.translate(oldpos.x() - newpos.x(), oldpos.y() - newpos.y());

        m_updateRect = currentBounds | oldBounds;
    }

    KisLayerOffsetCommand::~KisLayerOffsetCommand()
    {
    }

    void KisLayerOffsetCommand::execute()
    {
        moveTo(m_newPos);
    }

    void KisLayerOffsetCommand::unexecute()
    {
        moveTo(m_oldPos);
    }

    void KisLayerOffsetCommand::moveTo(const QPoint& pos)
    {
        if (m_layer->undoAdapter()) {
            m_layer->undoAdapter()->setUndo(false);
        }

        m_layer->setX(pos.x());
        m_layer->setY(pos.y());

        m_layer->setDirty(m_updateRect);

        if (m_layer->undoAdapter()) {
            m_layer->undoAdapter()->setUndo(true);
        }
    }
}

static int getID()
{
    static int id = 1;
    return id++;
}


KisLayer::KisLayer(KisImage *img, const QString &name, quint8 opacity) :
    QAbstractItemModel(0),
    KShared(),
    m_id(getID()),
    m_index(-1),
    m_opacity(opacity),
    m_locked(false),
    m_visible(true),
    m_temporary(false),
    m_name(name),
    m_parent(0),
    m_image(img),
    m_compositeOp(COMPOSITE_OVER)
{
    setObjectName(name);
}

KisLayer::KisLayer(const KisLayer& rhs) :
    QAbstractItemModel(0),
    KShared(rhs)
{
    if (this != &rhs) {
        m_id = getID();
        m_index = -1;
        m_opacity = rhs.m_opacity;
        m_locked = rhs.m_locked;
        m_visible = rhs.m_visible;
        m_temporary = rhs.m_temporary;
        m_dirtyRect = rhs.m_dirtyRect;
        m_name = rhs.m_name;
        m_image = rhs.m_image;
        m_parent = 0;
        m_compositeOp = rhs.m_compositeOp;
    }
}

KisLayer::~KisLayer()
{
}

void KisLayer::setClean(const QRect & rect)
{
    if (m_dirtyRect.isValid() && rect.isValid()) {

        // XXX: We should only set the parts clean that were actually cleaned. However, extent and exactBounds conspire
        // to make that very hard atm.
        //if (rect.contains(m_dirtyRect)) m_dirtyRect = QRect();
        m_dirtyRect = QRect();
    }

}

bool KisLayer::dirty()
{
    return m_dirtyRect.isValid();
}


bool KisLayer::dirty(const QRect & rc)
{
    if (!m_dirtyRect.isValid() || !rc.isValid()) return false;

    return rc.intersects(m_dirtyRect);
}

QRect KisLayer::dirtyRect() const
{
    return m_dirtyRect;
}

void KisLayer::setDirty(bool propagate)
{
    QRect rc = extent();

    if (rc.isValid()) m_dirtyRect = rc;

    // If we're dirty, our parent is dirty, if we've got a parent
    if (propagate && m_parent && rc.isValid()) m_parent->setDirty(m_dirtyRect);

    if (m_image && rc.isValid()) {
        m_image->notifyLayerUpdated(KisLayerSP(this), rc);
    }
}

void KisLayer::setDirty(const QRect & rc, bool propagate)
{
    // If we're dirty, our parent is dirty, if we've got a parent

    if (rc.isValid())
        m_dirtyRect |= rc;

    if (propagate && m_parent && m_dirtyRect.isValid())
        m_parent->setDirty(m_dirtyRect);

    if (m_image && rc.isValid()) {
        m_image->notifyLayerUpdated(KisLayerSP(this), rc);
    }
}

KisGroupLayerSP KisLayer::parent() const
{
    return m_parent;
}

KisLayerSP KisLayer::prevSibling() const
{
    if (!parent())
        return KisLayerSP(0);
    return parent()->at(index() - 1);
}

KisLayerSP KisLayer::nextSibling() const
{
    if (!parent())
        return KisLayerSP(0);
    return parent()->at(index() + 1);
}

int KisLayer::index() const
{
    return m_index;
}

void KisLayer::setIndex(int i)
{
    if (!parent())
        return;
    parent()->setIndex(KisLayerSP(this), i);
}

KisLayerSP KisLayer::findLayer(const QString& n) const
{
    if (name() == n)
        return KisLayerSP(const_cast<KisLayer*>(this)); //HACK any less ugly way? findLayer() is conceptually const...
    for (KisLayerSP layer = firstChild(); layer; layer = layer->nextSibling())
        if (KisLayerSP found = layer->findLayer(n))
            return found;
    return KisLayerSP(0);
}

KisLayerSP KisLayer::findLayer(int i) const
{
    if (id() == i)
        return KisLayerSP(const_cast<KisLayer*>(this)); //HACK
    for (KisLayerSP layer = firstChild(); layer; layer = layer->nextSibling())
        if (KisLayerSP found = layer->findLayer(i))
            return found;
    return KisLayerSP(0);
}

int KisLayer::numLayers(int flags) const
{
    int num = 0;
    if (matchesFlags(flags)) num++;
    for (KisLayerSP layer = firstChild(); layer; layer = layer->nextSibling())
        num += layer->numLayers(flags);
    return num;
}

bool KisLayer::matchesFlags(int flags) const
{
    if ((flags & Visible) && !visible())
        return false;
    if ((flags & Hidden) && visible())
        return false;
    if ((flags & Locked) && !locked())
        return false;
    if ((flags & Unlocked) && locked())
        return false;
    return true;
}

quint8 KisLayer::opacity() const
{
    return m_opacity;
}

void KisLayer::setOpacity(quint8 val)
{
    if (m_opacity != val)
    {
        m_opacity = val;
        setDirty();
        notifyPropertyChanged();
    }
}

KNamedCommand *KisLayer::setOpacityCommand(quint8 newOpacity)
{
    return new KisLayerOpacityCommand(KisLayerSP(this), opacity(), newOpacity);
}

KNamedCommand *KisLayer::setOpacityCommand(quint8 prevOpacity, quint8 newOpacity)
{
    return new KisLayerOpacityCommand(KisLayerSP(this), prevOpacity, newOpacity);
}

const bool KisLayer::visible() const
{
    return m_visible;
}

void KisLayer::setVisible(bool v)
{
    if (m_visible != v) {

        m_visible = v;
        notifyPropertyChanged();
        setDirty();

        if (undoAdapter() && undoAdapter()->undo()) {
            undoAdapter()->addCommand(setVisibleCommand(v));
        }
    }
}

KNamedCommand *KisLayer::setVisibleCommand(bool newVisibility)
{
    return new KisLayerVisibilityCommand(KisLayerSP(this), newVisibility);
}

bool KisLayer::locked() const
{
    return m_locked;
}

void KisLayer::setLocked(bool l)
{
    if (m_locked != l) {
        m_locked = l;
        notifyPropertyChanged();

        if (undoAdapter() && undoAdapter()->undo()) {
            undoAdapter()->addCommand(setLockedCommand(l));
        }
    }
}

bool KisLayer::temporary() const
{
    return m_temporary;
}

void KisLayer::setTemporary(bool t)
{
    m_temporary = t;
}

KNamedCommand *KisLayer::setLockedCommand(bool newLocked)
{
    return new KisLayerLockedCommand(KisLayerSP(this), newLocked);
}

QString KisLayer::name() const
{
        return m_name;
}

void KisLayer::setName(const QString& name)
{
    if (!name.isEmpty() && m_name != name)
    {
        m_name = name;
        notifyPropertyChanged();
    }
}

void KisLayer::setCompositeOp(const KoCompositeOp& compositeOp)
{
    if (m_compositeOp != compositeOp)
    {
       m_compositeOp = compositeOp;
       notifyPropertyChanged();
       setDirty();

    }
}

KNamedCommand *KisLayer::setCompositeOpCommand(const KoCompositeOp& newCompositeOp)
{
    return new KisLayerCompositeOpCommand(KisLayerSP(this), compositeOp(), newCompositeOp);
}

KNamedCommand *KisLayer::moveCommand(QPoint oldPosition, QPoint newPosition)
{
    return new KisLayerOffsetCommand(KisLayerSP(this), oldPosition, newPosition);
}

KisUndoAdapter *KisLayer::undoAdapter() const
{
    if (m_image) {
        return m_image->undoAdapter();
    }
    return 0;
}

void KisLayer::paintMaskInactiveLayers(QImage &, qint32, qint32, qint32, qint32)
{
}

void KisLayer::paintSelection(QImage &, qint32, qint32, qint32, qint32)
{
}

void KisLayer::paintSelection(QImage &, const QRect&, const QSize&, const QSize&)
{
}

QImage KisLayer::createThumbnail(qint32, qint32)
{
    return QImage();
}

void KisLayer::notifyPropertyChanged()
{
    if(image() && !signalsBlocked())
        image()->notifyPropertyChanged(KisLayerSP(this));
}

int KisLayer::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 1;
    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    return static_cast<KisLayer*>(parent.internalPointer())->childCount();
}

int KisLayer::columnCount(const QModelIndex&) const
{
    return 0;
}

QModelIndex KisLayer::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        Q_ASSERT(row == 0);
        return createIndex(row, column, (void*)this);
    }

    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    return createIndex(row, column, static_cast<KisLayer*>(parent.internalPointer())->at(row));
}

QModelIndex KisLayer::parent(const QModelIndex &i) const
{
    if (!i.isValid())
        return QModelIndex();
    Q_ASSERT(i.model() == this);
    Q_ASSERT(i.internalPointer());

    if (KisGroupLayer *p = static_cast<KisLayer*>(i.internalPointer())->parent().data())
        return createIndex(p->KisLayer::index(), 0, p); //gcc--
    else
        return QModelIndex();
}

QVariant KisLayer::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisLayer *layer = static_cast<KisLayer*>(index.internalPointer());

    switch (role)
    {
        case Qt::DisplayRole: return layer->name();
        case Qt::DecorationRole: return layer->icon();
        case Qt::EditRole: return layer->name();
        case ThumbnailRole: return layer->createThumbnail(64, 64);
        default: return QVariant(); //TODO
    }
}



#include "kis_layer.moc"

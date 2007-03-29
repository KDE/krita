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
#include <kicon.h>
#include <QIcon>
#include <QImage>
#include <QBitArray>

#include "kis_debug_areas.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_painter.h"


static int getID()
{
    static int id = 1;
    return id++;
}

class KisLayer::Private {

public:

    int id;
    int index;
    quint8 opacity;
    bool locked;
    bool visible;
    bool temporary;

    QString name;
    KisGroupLayerSP parent;
    KisImageSP image;
    QBitArray channelFlags;

    // Operation used to composite this layer with the projection of
    // the layers _under_ this layer
    const KoCompositeOp * compositeOp;
#ifdef DIRTY_AND_PROJECTION
    QRegion dirtyRegion; // XXX: this should be part of the data manager!
#endif
};


KisLayer::KisLayer(KisImageSP img, const QString &name, quint8 opacity)
    : KoDocumentSectionModel(0)
    , m_d( new Private )
{
    m_d->id = getID();
    m_d->index = -1;
    m_d->opacity = opacity;
    m_d->locked = false;
    m_d->visible = true;
    m_d->temporary = false;
    m_d->name = name;
    m_d->parent = 0;
    m_d->image = img;
    m_d->compositeOp = const_cast<KoCompositeOp*>( img->colorSpace()->compositeOp( COMPOSITE_OVER ) );
    setObjectName(name);
}

KisLayer::KisLayer(const KisLayer& rhs)
    : KoDocumentSectionModel( 0 )
    , KisShared(rhs)
    , m_d( new Private() )
{
    if (this != &rhs) {
        m_d->id = getID();
        m_d->index = -1;
        m_d->opacity = rhs.m_d->opacity;
        m_d->locked = rhs.m_d->locked;
        m_d->visible = rhs.m_d->visible;
        m_d->temporary = rhs.m_d->temporary;
        m_d->name = rhs.m_d->name;
        m_d->image = rhs.m_d->image;
        m_d->parent = 0;
        m_d->compositeOp = rhs.m_d->compositeOp;
    }
}

KisLayer::~KisLayer()
{
}

KoColorSpace * KisLayer::colorSpace()
{
    return m_d->image->colorSpace();
}

KoDocumentSectionModel::PropertyList KisLayer::properties() const
{
    PropertyList l;
    l << Property(i18n("Visible"), KIcon("visible"), KIcon("novisible"), visible());
    l << Property(i18n("Locked"), KIcon("locked"), KIcon("unlocked"), locked());
    l << Property(i18n("Opacity"), i18n("%1%", percentOpacity()));
    l << Property(i18n("Composite Mode"), compositeOp()->id());
    return l;
}

void KisLayer::setProperties( const PropertyList &properties )
{
    setVisible( properties.at( 0 ).state.toBool() );
    setLocked( properties.at( 1 ).state.toBool() );
}

void KisLayer::setChannelFlags( QBitArray & channelFlags )
{
    Q_ASSERT( ( ( quint32 )channelFlags.count() == colorSpace()->channelCount() || channelFlags.isEmpty()) );
    m_d->channelFlags = channelFlags;
}

QBitArray & KisLayer::channelFlags()
{
    return m_d->channelFlags;
}


void KisLayer::activate()
{
    notifyPropertyChanged(this);
}

void KisLayer::deactivate()
{
    /*notifyPropertyChanged(this); not necessary, and causes scrolling issues */
}

bool KisLayer::isActive() const
{
    if (image())
        return this == image()->activeLayer().data();
    else
        return false;
}

void KisLayer::setActive()
{
    if (image())
        image()->activateLayer(KisLayerSP(this));
}



void KisLayer::setDirty()
{
    QRect rc = extent();
    setDirty( QRegion( rc ) );
}

void KisLayer::setDirty(const QRect & rc)
{
    setDirty( QRegion( rc ) );
}

void KisLayer::setDirty( const QRegion & region)
{
    // If we're dirty, our parent is dirty, if we've got a parent
    if ( region.isEmpty() ) return;

    if (m_d->parent) {
        m_d->parent->setDirty(region);
    }
    if (m_d->image.data()) {
        m_d->image->notifyLayerUpdated(KisLayerSP(this));
    }
#ifdef DIRTY_AND_PROJECTION
    m_d->dirtyRegion += region;
#endif
}

#ifdef DIRTY_AND_PROJECTION
bool KisLayer::isDirty( const QRect & rect )
{
    return m_d->dirtyRegion.intersects( rect );
}

void KisLayer::setClean( QRect rc )
{
    m_d->dirtyRegion -= QRegion( rc );
}
#endif

int KisLayer::id() const { return m_d->id; }

KisGroupLayerSP KisLayer::parent() const
{
    return m_d->parent;
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
    return m_d->index;
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
    for (KisLayer* layer = firstChild().data(); layer; layer = layer->nextSibling().data())
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

KisLayerSP KisLayer::layerFromIndex(const QModelIndex &index)
{
    if( !index.isValid() )
        return KisLayerSP(0);

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    return KisLayerSP(static_cast<KisLayer*>(index.internalPointer()));
}

vKisLayerSP KisLayer::layersFromIndexes(const QModelIndexList &in)
{
    vKisLayerSP out;
    for (int i = 0, n = in.count(); i < n; ++i)
        if (KisLayerSP layer = layerFromIndex(in.at(i)))
            out << layer;
    return out;
}

quint8 KisLayer::opacity() const
{
    return m_d->opacity;
}

void KisLayer::setOpacity(quint8 val)
{
    if (m_d->opacity != val)
    {
        m_d->opacity = val;
        setDirty();
        notifyPropertyChanged();
    }
}

quint8 KisLayer::percentOpacity() const
{
    return int(float(opacity() * 100) / 255 + 0.5);
}

void KisLayer::setPercentOpacity(quint8 val)
{
    setOpacity(int(float(val * 255) / 100 + 0.5));
}

const bool KisLayer::visible() const
{
    return m_d->visible;
}

void KisLayer::setVisible(bool v)
{
    if (m_d->visible != v) {

        m_d->visible = v;
        notifyPropertyChanged();
        setDirty();
    }
}

bool KisLayer::locked() const
{
    return m_d->locked;
}

void KisLayer::setLocked(bool l)
{
    if (m_d->locked != l) {
        m_d->locked = l;
        notifyPropertyChanged();
    }
}

bool KisLayer::temporary() const
{
    return m_d->temporary;
}

void KisLayer::setTemporary(bool t)
{
    m_d->temporary = t;
}

QString KisLayer::name() const
{
        return m_d->name;
}

void KisLayer::setName(const QString& name)
{
    if (!name.isEmpty() && m_d->name != name)
    {
        m_d->name = name;
        notifyPropertyChanged();
    }
}

const KoCompositeOp * KisLayer::compositeOp() const
{
    return m_d->compositeOp;
}

void KisLayer::setCompositeOp(const KoCompositeOp* compositeOp)
{
    if (m_d->compositeOp != compositeOp)
    {
       m_d->compositeOp = const_cast<KoCompositeOp*>( compositeOp );
       notifyPropertyChanged();
       setDirty();
    }
}

KisImageSP KisLayer::image() const { return m_d->image; }

void KisLayer::setImage(KisImageSP image)
{
    m_d->image = image;
}

void KisLayer::paintMaskInactiveLayers(QImage &, qint32, qint32, qint32, qint32)
{
}

void KisLayer::paint(QImage &, qint32, qint32, qint32, qint32)
{
}

void KisLayer::paint(QImage &, const QRect&, const QSize&, const QSize&)
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
    notifyPropertyChanged(this);
}

void KisLayer::notifyCommandExecuted()
{
    notifyPropertyChanged(this);
}

void KisLayer::notifyPropertyChanged(KisLayer *layer)
{
    QModelIndex index = indexFromLayer(layer);
    emit dataChanged(index, index);
    if (parent())
        parent()->notifyPropertyChanged(layer); // To make sure the
                                                // group layers
                                                // thumbnails are
                                                // updated, too.
}

QModelIndex KisLayer::indexFromLayer(KisLayer *layer) const
{
    Q_ASSERT(layer);
    return createIndex(layer->index(), 0, layer);
}

int KisLayer::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return childCount();
    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    return static_cast<KisLayer*>(parent.internalPointer())->childCount();
}

int KisLayer::columnCount(const QModelIndex&) const
{
    return 1;
}

QModelIndex KisLayer::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        if( static_cast<uint>( row ) < childCount() )
            return createIndex(row, column, at(row).data());
        else
            return QModelIndex();
    }

    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    return createIndex(row, column, static_cast<KisLayer*>(parent.internalPointer())->at(row).data());
}

QModelIndex KisLayer::parent(const QModelIndex &i) const
{
    if (!i.isValid())
        return QModelIndex();
    Q_ASSERT(i.model() == this);
    Q_ASSERT(i.internalPointer());

    if (static_cast<KisLayer*>(i.internalPointer())->parent().data() == this)
        return QModelIndex();
    else if (KisGroupLayer *p = static_cast<KisLayer*>(i.internalPointer())->parent().data())
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
        case Qt::SizeHintRole: return layer->image()->size();
        case ActiveRole: return layer->isActive();
        case PropertiesRole: return QVariant::fromValue(layer->properties());
        case AspectRatioRole: return double(layer->image()->width()) / layer->image()->height();
        default:
            if (role >= int(BeginThumbnailRole))
                return layer->createThumbnail(role - int(BeginThumbnailRole), role - int(BeginThumbnailRole));
            else
                return QVariant();
    }
}

Qt::ItemFlags KisLayer::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
    if (qobject_cast<KisGroupLayer*>(static_cast<KisLayer*>(index.internalPointer()))) //gcc--
        flags |= Qt::ItemIsDropEnabled;
    return flags;
}

bool KisLayer::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisLayer *layer = static_cast<KisLayer*>(index.internalPointer());

    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
            layer->setName(value.toString());
            return true;
        case PropertiesRole:
            layer->setProperties(value.value<PropertyList>());
            return true;
        case ActiveRole:
            if (value.toBool())
            {
                layer->setActive();
                return true;
            }
    }

    return false;
}


void KisLayer::setIndexPrivate( int index )
{
    m_d->index = index;
}

void KisLayer::setCompositeOpPrivate( const KoCompositeOp * op )
{
    m_d->compositeOp = op;
}

void KisLayer::setParentPrivate( KisGroupLayerSP parent )
{
    m_d->parent = parent;
}

void KisIndirectPaintingSupport::setTemporaryTarget(KisPaintDeviceSP t) {
    m_temporaryTarget = t;
}

void KisIndirectPaintingSupport::setTemporaryCompositeOp(const KoCompositeOp* c) {
    m_compositeOp = c;
}

void KisIndirectPaintingSupport::setTemporaryOpacity(Q_UINT8 o) {
    m_compositeOpacity = o;
}

KisPaintDeviceSP KisIndirectPaintingSupport::temporaryTarget() {
    return m_temporaryTarget;
}

const KoCompositeOp* KisIndirectPaintingSupport::temporaryCompositeOp() const {
    return m_compositeOp;
}

Q_UINT8 KisIndirectPaintingSupport::temporaryOpacity() const {
    return m_compositeOpacity;
}


#include "kis_layer.moc"

/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <QIcon>
#include <QImage>
#include <QBitArray>

#include <KoProperties.h>

#include "kis_debug_areas.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_painter.h"

#include "kis_mask.h"
#include "kis_effect_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "kis_meta_data_store.h"

class KisLayer::Private {

public:

    KisImageWSP image;
    QBitArray channelFlags;
    const KoCompositeOp * compositeOp;
    KisEffectMaskSP previewMask;
    KisMetaData::Store* metaDataStore;
};


KisLayer::KisLayer(KisImageWSP img, const QString &name, quint8 opacity)
    : KisNode()
    , m_d( new Private )
{
//     Q_ASSERT( img );
    setName( name );
    setOpacity( opacity );
    m_d->image = img;
	if (m_d->image)
    	m_d->compositeOp = const_cast<KoCompositeOp*>( img->colorSpace()->compositeOp( COMPOSITE_OVER ) );
	else
		m_d->compositeOp = 0;
    m_d->metaDataStore = new KisMetaData::Store();
}

KisLayer::KisLayer(const KisLayer& rhs)
    : KisNode( rhs )
    , m_d( new Private() )
{
    if (this != &rhs) {
        m_d->image = rhs.m_d->image;
        m_d->compositeOp = rhs.m_d->compositeOp;
        m_d->metaDataStore = new KisMetaData::Store(*rhs.m_d->metaDataStore);
    }
}

KisLayer::~KisLayer()
{
    delete m_d->metaDataStore;
    delete m_d;
}

KoColorSpace * KisLayer::colorSpace()
{
	if (m_d->image)
    	return m_d->image->colorSpace();
	return 0;
}

KoDocumentSectionModel::PropertyList KisLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisBaseNode::sectionModelProperties();
    l << KoDocumentSectionModel::Property(i18n("Opacity"), i18n("%1%", percentOpacity()));
	if (compositeOp())
    	l << KoDocumentSectionModel::Property(i18n("Composite Mode"), compositeOp()->id());
    return l;
}

void KisLayer::setSectionModelProperties( const KoDocumentSectionModel::PropertyList &properties )
{
    setOpacity( properties.at( 2 ).state.toInt() );
    setCompositeOp( const_cast<KoCompositeOp*>( image()->colorSpace()->compositeOp( properties.at( 3 ).state.toString() ) ) );
}

KisPaintDeviceSP KisLayer::original() const
{
    return projection();
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

quint8 KisLayer::opacity() const
{
    return nodeProperties().intProperty( "opacity", OPACITY_OPAQUE );
}

void KisLayer::setOpacity(quint8 val)
{
    if ( opacity() != val )
    {
        nodeProperties().setProperty( "opacity", val );
        setDirty();
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

bool KisLayer::temporary() const
{
    return nodeProperties().boolProperty( "temporary", false );
}

void KisLayer::setTemporary(bool t)
{
    nodeProperties().setProperty( "temporary", t );
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
    }
}

KisImageSP KisLayer::image() const
{
    return m_d->image;
}

void KisLayer::setImage(KisImageSP image)
{
    m_d->image = image;
    for ( uint i = 0; i < childCount(); ++i ) {
        // Only layers know about the image
        KisLayer * layer = dynamic_cast<KisLayer*>( at( i ).data() );
        if ( layer )
            layer->setImage( image );
    }
}

void KisLayer::paint(QImage &, qint32, qint32, qint32, qint32)
{
}

void KisLayer::paint(QImage &, const QRect&, const QSize&, const QSize&)
{
}

KisSelectionMaskSP KisLayer::selectionMask() const
{
    return 0;
}

bool KisLayer::hasEffectMasks() const
{
    QList<KisNodeSP> masks = childNodes( QStringList( "KisEffectMask" ), KoProperties() );

    // If all these things don't exist, we have no effectMasks.
    return !(  m_d->previewMask == 0 && masks.isEmpty() );
}

void KisLayer::applyEffectMasks( const KisPaintDeviceSP projection, const QRect & rc )
{
    if ( isDirty(rc) ) {

        QList<KisNodeSP> masks = childNodes( QStringList( "KisEffectMask" ), KoProperties() );

        // Then loop through the effect masks and apply them
        for ( int i = 0; i < masks.size(); ++i ) {

            const KisEffectMask * effectMask = dynamic_cast<const KisEffectMask*>( masks.at( i ).data() );

            if ( effectMask )
                effectMask->apply( projection, rc );
        }

        // Then apply the preview mask
        if ( m_d->previewMask )
            m_d->previewMask->apply( projection, rc );

    }
}


QList<KisMaskSP> KisLayer::effectMasks() const
{
    QList<KisNodeSP> nodes = childNodes( QStringList( "KisEffectMask" ), KoProperties() );
    QList<KisMaskSP> masks;
    foreach( KisNodeSP node,  nodes ) {
        KisMaskSP mask = dynamic_cast<KisMask*>( node.data() );
        if ( mask )
            masks.append( mask );
    }
    return masks;

}

void KisLayer::setPreviewMask( KisEffectMaskSP mask )
{
    m_d->previewMask = mask;
    setDirty( mask->extent() );
}

KisEffectMaskSP KisLayer::previewMask() const
{
    return m_d->previewMask;
}

void KisLayer::removePreviewMask()
{
    m_d->previewMask = 0;
    if ( m_d->previewMask ) setDirty( m_d->previewMask->extent() );
}

KisLayerSP KisLayer::parentLayer() const
{
    return dynamic_cast<KisLayer*>( parent().data() );
}

KisMetaData::Store* KisLayer::metaData()
{
    return m_d->metaDataStore;
}

void KisIndirectPaintingSupport::setTemporaryTarget(KisPaintDeviceSP t) {
    m_temporaryTarget = t;
}

void KisIndirectPaintingSupport::setTemporaryCompositeOp(const KoCompositeOp* c) {
    m_compositeOp = c;
}

void KisIndirectPaintingSupport::setTemporaryOpacity(quint8 o) {
    m_compositeOpacity = o;
}

KisPaintDeviceSP KisIndirectPaintingSupport::temporaryTarget() {
    return m_temporaryTarget;
}

const KoCompositeOp* KisIndirectPaintingSupport::temporaryCompositeOp() const {
    return m_compositeOp;
}

quint8 KisIndirectPaintingSupport::temporaryOpacity() const {
    return m_compositeOpacity;
}


KisIndirectPaintingSupport::KisIndirectPaintingSupport() : m_compositeOp(0) { }
KisIndirectPaintingSupport::~KisIndirectPaintingSupport() {}

#include "kis_layer.moc"

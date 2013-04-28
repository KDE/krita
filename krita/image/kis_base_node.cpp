/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_base_node.h"
#include <klocale.h>

#include <KoIcon.h>
#include <KoProperties.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include "kis_paint_device.h"

struct KisBaseNode::Private
{
public:

    QString compositeOp;
    KoProperties properties;
    KisBaseNodeSP linkedTo;
    bool systemLocked;
    KoDocumentSectionModel::Property hack_visible; //HACK
    QUuid id;
    bool collapsed;
};

KisBaseNode::KisBaseNode()
    : m_d(new Private())
{
    /**
     * Be cautions! These two calls are vital to warm-up KoProperties.
     * We use it and its QMap in a threaded environment. This is not
     * officially suported by Qt, but our environment guarantees, that
     * there will be the only writer and several readers. Whilst the
     * value of the QMap is boolean and there are no implicit-sharing
     * calls provocated, it is safe to work with it in such an
     * environment.
     */
    setVisible(true);
    setUserLocked(false);
    setCollapsed(false);

    setSystemLocked(false);
    m_d->linkedTo = 0;
    m_d->compositeOp = COMPOSITE_OVER;
    
    setUuid(QUuid::createUuid());
}


KisBaseNode::KisBaseNode(const KisBaseNode & rhs)
    : QObject()
    , KisShared()
    ,  m_d(new Private())
{
    QMapIterator<QString, QVariant> iter = rhs.m_d->properties.propertyIterator();
    while (iter.hasNext()) {
        iter.next();
        m_d->properties.setProperty(iter.key(), iter.value());
    }
    m_d->linkedTo = rhs.m_d->linkedTo;
    m_d->compositeOp = rhs.m_d->compositeOp;
    
    setUuid(QUuid::createUuid());
}

KisBaseNode::~KisBaseNode()
{
    delete m_d;
}

KisPaintDeviceSP KisBaseNode::paintDevice() const
{
    return 0;
}

KisPaintDeviceSP KisBaseNode::original() const
{
    return 0;
}

KisPaintDeviceSP KisBaseNode::projection() const
{
    return 0;
}

quint8 KisBaseNode::opacity() const
{
    return nodeProperties().intProperty("opacity", OPACITY_OPAQUE_U8);
}

void KisBaseNode::setOpacity(quint8 val)
{
    if (opacity() != val) {
        nodeProperties().setProperty("opacity", val);
    }
    baseNodeChangedCallback();
}

quint8 KisBaseNode::percentOpacity() const
{
    return int(float(opacity() * 100) / 255 + 0.5);
}

void KisBaseNode::setPercentOpacity(quint8 val)
{
    setOpacity(int(float(val * 255) / 100 + 0.5));
}

const QString& KisBaseNode::compositeOpId() const
{
    return m_d->compositeOp;
}

/**
 * FIXME: Rename this function to setCompositeOpId()
 */
void KisBaseNode::setCompositeOp(const QString& compositeOp)
{
    m_d->compositeOp = compositeOp;
    baseNodeChangedCallback();
}

KoDocumentSectionModel::PropertyList KisBaseNode::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l;
    l << KoDocumentSectionModel::Property(i18n("Visible"), koIcon("visible"), koIcon("novisible"), visible(), m_d->hack_visible.isInStasis, m_d->hack_visible.stateInStasis);
    l << KoDocumentSectionModel::Property(i18n("Locked"), koIcon("locked"), koIcon("unlocked"), userLocked());
    return l;
}

void KisBaseNode::setSectionModelProperties(const KoDocumentSectionModel::PropertyList &properties)
{
    setVisible(properties.at(0).state.toBool());
    m_d->hack_visible = properties.at(0);
    setUserLocked(properties.at(1).state.toBool());
}

KoProperties & KisBaseNode::nodeProperties() const
{
    return m_d->properties;
}

void KisBaseNode::mergeNodeProperties(const KoProperties & properties)
{
    QMapIterator<QString, QVariant> iter = properties.propertyIterator();
    while (iter.hasNext()) {
        iter.next();
        m_d->properties.setProperty(iter.key(), iter.value());
    }
    baseNodeChangedCallback();
}

bool KisBaseNode::check(const KoProperties & properties) const
{
    QMapIterator<QString, QVariant> iter = properties.propertyIterator();
    while (iter.hasNext()) {
        iter.next();
        if (m_d->properties.contains(iter.key())) {
            if (m_d->properties.value(iter.key()) != iter.value())
                return false;
        }
    }
    return true;
}


QImage KisBaseNode::createThumbnail(qint32 w, qint32 h)
{
    try {
        QImage image(w, h, QImage::Format_ARGB32);
        image.fill(0);
        return image;
    } catch (std::bad_alloc) {
        return QImage();
    }

}

bool KisBaseNode::visible(bool recursive) const
{
    bool isVisible = m_d->properties.boolProperty("visible", true);
    KisBaseNodeSP parentNode = parentCallback();

    return recursive && isVisible && parentNode ?
        parentNode->visible() : isVisible;
}

void KisBaseNode::setVisible(bool visible)
{
    m_d->properties.setProperty("visible", visible);
    emit visibilityChanged(visible);
    baseNodeChangedCallback();
}

bool KisBaseNode::userLocked() const
{
    return m_d->properties.boolProperty("locked", false);
}

void KisBaseNode::setUserLocked(bool locked)
{
    m_d->properties.setProperty("locked", locked);
    emit userLockingChanged(locked);
    baseNodeChangedCallback();
}

bool KisBaseNode::systemLocked() const
{
    return m_d->systemLocked;
}

void KisBaseNode::setSystemLocked(bool locked, bool update)
{
    m_d->systemLocked = locked;
    if (update) {
        emit systemLockingChanged(locked);
        baseNodeChangedCallback();
    }
}

bool KisBaseNode::isEditable() const
{
    bool editable = (m_d->properties.boolProperty("visible", true) && !userLocked() && !systemLocked());

    if (editable) {
        KisBaseNodeSP parentNode = parentCallback();
        if (parentNode && parentNode != this) {
            editable = parentNode->isEditable();
        }
    }
    return editable;
}

void KisBaseNode::setCollapsed(bool collapsed)
{
    m_d->collapsed = collapsed;
}

bool KisBaseNode::collapsed() const
{
    return m_d->collapsed;
}

QUuid KisBaseNode::uuid() const
{
    return m_d->id;
}

void KisBaseNode::setUuid(const QUuid& id)
{
    m_d->id = id;
    baseNodeChangedCallback();
}

#include "kis_base_node.moc"

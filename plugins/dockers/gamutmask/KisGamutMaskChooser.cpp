/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisGamutMaskChooser.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QAbstractItemDelegate>

#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoResourceItemChooser.h>
#include <KoResourceServerAdapter.h>


/// The resource item delegate for rendering the resource preview
class KisGamutMaskDelegate: public QAbstractItemDelegate
{
public:
    KisGamutMaskDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent) {}
    ~KisGamutMaskDelegate() override {}
    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        return option.decorationSize;
    }
};

void KisGamutMaskDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!index.isValid())
        return;

    KoResource* resource = static_cast<KoResource*>(index.internalPointer());
    KoGamutMask* mask = static_cast<KoGamutMask*>(resource);

    if (!mask) {
        return;
    }

    QImage preview = mask->image();

    if(preview.isNull()) {
        return;
    }

    QRect paintRect = option.rect.adjusted(1, 1, -1, -1);
    painter->drawImage(paintRect.x(), paintRect.y(),
                       preview.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    painter->restore();
}


KisGamutMaskChooser::KisGamutMaskChooser(QWidget *parent) : QWidget(parent)
{
    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    QSharedPointer<KoAbstractResourceServerAdapter> adapter(new KoResourceServerAdapter<KoGamutMask>(rServer));
    m_itemChooser = new KoResourceItemChooser(adapter, this);
    m_itemChooser->setItemDelegate(new KisGamutMaskDelegate(this));
    m_itemChooser->showTaggingBar(true);
    m_itemChooser->showButtons(false);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    // TODO: menu for view mode change

    layout->addWidget(m_itemChooser);
    setLayout(layout);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource*)), this, SLOT(resourceSelected(KoResource*)));
}

KisGamutMaskChooser::~KisGamutMaskChooser()
{

}

void KisGamutMaskChooser::setCurrentResource(KoResource *resource)
{
    m_itemChooser->setCurrentResource(resource);
}

void KisGamutMaskChooser::resourceSelected(KoResource* resource)
{
    emit sigGamutMaskSelected(static_cast<KoGamutMask*>(resource));
}

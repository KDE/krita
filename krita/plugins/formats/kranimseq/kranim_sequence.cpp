/*
 *  Copyright (c) 2014 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kranim_sequence.h"
#include "sequence_generator.h"

#include <QCheckBox>
#include <QSlider>

#include <kapplication.h>
#include <kdialog.h>
#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoFilterChain.h>
#include <KoFilterManager.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>

#include "kis_paint_device.h"
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_config.h"
#include "kis_properties_configuration.h"
#include "kis_meta_data_store.h"
#include "kis_meta_data_filter_registry_model.h"
#include "kis_exif_info_visitor.h"
#include "kis_png_converter.h"
#include "kis_iterator_ng.h"
#include "kis_animation_doc.h"

K_PLUGIN_FACTORY(KranimSequenceFactory, registerPlugin<KranimSequence>();)
K_EXPORT_PLUGIN(KranimSequenceFactory("calligrafilters"))

KranimSequence::KranimSequence(QObject *parent, const QVariantList &) : KoFilter(parent)
{

}

KranimSequence::~KranimSequence()
{

}

bool hasVisibleWidgets()
{
    QWidgetList wl = QApplication::allWidgets();
    foreach(QWidget* w, wl) {
        if (w->isVisible() && strcmp(w->metaObject()->className(), "QDesktopWidget")) {
            dbgFile << "Widget " << w << " " << w->objectName() << " " << w->metaObject()->className() << " is visible";
            return true;
        }
    }
    return false;
}

KoFilter::ConversionStatus KranimSequence::convert(const QByteArray &from, const QByteArray &to)
{
    KDialog* kdb = new KDialog(0);
    kdb->setCaption(i18n("Animation Sequence export options"));
    kdb->setModal(false);
    kdb->setMinimumWidth(300);

    m_wdg = new KisWdgOptionsKranimseq(kdb);
    kdb->setMainWidget(m_wdg);

    if(kdb->exec() == QDialog::Accepted) {
        KisAnimationDoc* doc = dynamic_cast<KisAnimationDoc*>(m_chain->inputDocument());
        QString filename = m_chain->outputFile();
        SequenceGenerator* generator = new SequenceGenerator(doc, filename);

        bool keyFrameOnly = m_wdg->keyFrameOnly->isChecked();
        int startFrame = m_wdg->startFrameInput->value();
        int endFrame = m_wdg->stopFrameInput->value();

        if(generator->generate(keyFrameOnly, startFrame, endFrame)) {
            return KoFilter::OK;
        }
    }

    return KoFilter::InternalError;
}

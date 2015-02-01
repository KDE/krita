/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_ocio_display_filter_test.h"

#include <qtest_kde.h>

#include <QFile>

#include "KoColorModelStandardIds.h"

#include "../../../../../sdk/tests/stroke_testing_utils.h"
#include "../../../../../sdk/tests/testutil.h"

#include "kis_exposure_gamma_correction_interface.h"
#include "../ocio_display_filter.h"
#include "kis_display_color_converter.h"
#include "kis_canvas_resource_provider.h"

#include <KoChannelInfo.h>


void KisOcioDisplayFilterTest::test()
{
    KisExposureGammaCorrectionInterface *egInterface =
        new KisDumbExposureGammaCorrectionInterface();

    OcioDisplayFilter filter(egInterface);

    QString configFile = TestUtil::fetchDataFileLazy("./psyfiTestingConfig-master/config.ocio");
    qDebug() << ppVar(configFile);

    Q_ASSERT(QFile::exists(configFile));

    OCIO::ConstConfigRcPtr ocioConfig =
        OCIO::Config::CreateFromFile(configFile.toUtf8());

    filter.config = ocioConfig;
    filter.inputColorSpaceName = ocioConfig->getColorSpaceNameByIndex(0);
    filter.displayDevice = ocioConfig->getDisplay(1);
    filter.view = ocioConfig->getView(filter.displayDevice, 0);
    filter.gamma = 1.0;
    filter.exposure = 0.0;
    filter.swizzle = RGBA;

    filter.blackPoint = 0.0;
    filter.whitePoint = 1.0;

    filter.forceInternalColorManagement = false;
    filter.setLockCurrentColorVisualRepresentation(false);

    filter.updateProcessor();

    qDebug() << ppVar(filter.inputColorSpaceName);
    qDebug() << ppVar(filter.displayDevice);
    qDebug() << ppVar(filter.view);
    qDebug() << ppVar(filter.gamma);
    qDebug() << ppVar(filter.exposure);

    const KoColorSpace *paintingCS =
                KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);

    KisImageSP image = utils::createImage(0, QSize(100, 100));
    image->convertImageColorSpace(paintingCS, KoColorConversionTransformation::InternalRenderingIntent, KoColorConversionTransformation::InternalConversionFlags);
    image->waitForDone();

qDebug() << ppVar(paintingCS) << ppVar(image->root()->firstChild()->colorSpace());

    KoCanvasResourceManager *resourceManager =
        utils::createResourceManager(image,
                                     image->root(), "");

    KisDisplayColorConverter converter(resourceManager, 0);

    qDebug() << ppVar(image->root()->firstChild());

    QVariant v;
    v.setValue(KisNodeWSP(image->root()->firstChild()));
    resourceManager->setResource(KisCanvasResourceProvider::CurrentKritaNode, v);

    converter.setDisplayFilter(&filter);
    qDebug() << ppVar(converter.paintingColorSpace());

    {
        QColor refColor(255, 128, 0);
        KoColor realColor = converter.approximateFromRenderedQColor(refColor);
        QColor roundTripColor = converter.toQColor(realColor);

        qDebug() << ppVar(refColor);
        qDebug() << ppVar(realColor.colorSpace()) << ppVar(KoColor::toQString(realColor));
        qDebug() << ppVar(roundTripColor);
    }

    {
        KoColor realColor(Qt::red, paintingCS);
        QColor roundTripColor = converter.toQColor(realColor);

        qDebug() << ppVar(realColor.colorSpace()) << ppVar(KoColor::toQString(realColor));
        qDebug() << ppVar(roundTripColor);
    }

}

QTEST_KDEMAIN(KisOcioDisplayFilterTest, GUI)

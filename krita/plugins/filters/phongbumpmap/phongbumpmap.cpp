/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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

#include "phongbumpmap.h"

#include <stdlib.h>

#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QPoint>
#include <QPushButton>
#include <QString>

#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <KoColorTransformation.h>
#include <KoIntegerMaths.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoDocumentSectionView.h>

#include <kis_debug.h>
#include <kis_doc2.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <kis_node_model.h>

#include <QPixmap>
#include <QImage>
#include <QDebug>
#include <QTime>
#include <QVector3D>
#include <QVector>
#include <cmath>
#include <iostream>

#include <time.h>

#include <KoColorSpaceRegistry.h>
#include <colorprofiles/KoDummyColorProfile.h>

#include <QColor>

#include <opengl/kis_opengl.h>
/*
#include <../../extensions/impasto/kis_fresh_start_color_space.h>
#include <../../extensions/impasto/kis_fresh_start_color_space.cpp>
*/

#include "kdebug.h"
#include <kis_painter.h>

K_PLUGIN_FACTORY(KritaPhongBumpmapFactory, registerPlugin<KritaPhongBumpmap>();)
K_EXPORT_PLUGIN(KritaPhongBumpmapFactory("krita"))


KritaPhongBumpmap::KritaPhongBumpmap(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterPhongBumpmap());
}

KritaPhongBumpmap::~KritaPhongBumpmap()
{
}

void KisFilterPhongBumpmap::process(KisPaintDeviceSP device,
                                    const QRect& applyRect,
                                    const KisFilterConfiguration* config,
                                    KoUpdater* /*progressUpdater*/
                                    ) const
{
#ifdef __GNUC__
    #warning TODO: implement progress updater for phong bumpmap
#endif

    // Benchmark
    QTime timer, timerE;

    QString userChosenHeightChannel = config->getString(PHONG_HEIGHT_CHANNEL, "FAIL");

    if (userChosenHeightChannel == "FAIL") {
        qDebug("FIX YOUR FILTER");
        return;
    }

    timer.start();

    KoChannelInfo* m_heightChannel = 0;

    foreach (KoChannelInfo* channel, device->colorSpace()->channels()) {
        if (userChosenHeightChannel == channel->name())
            m_heightChannel = channel;
    }

    qDebug("Tiempo de preparacion: %d ms", timer.restart());

    QRect inputArea = applyRect;
    inputArea.adjust(-1, -1, 1, 1);
    QRect outputArea = inputArea.adjusted(1, 1, -1, -1);
    //QRect outputArea(dstInfo.topLeft().x()+1, dstInfo.topLeft().y()+1, size.width()-1, size.height()-1);

    //qDebug() << "inputArea: " << inputArea << srcInfo.topLeft();
    //qDebug() << "outputArea: " << outputArea << dstInfo.topLeft();
    quint32 posup;
    quint32 posdown;
    quint32 posleft;
    quint32 posright;
    QRect tileLimits;

    QColor I; //Reflected light

    //======================================
    //======Preparation paraphlenalia=======
    //======================================

    QImage bumpmap(outputArea.width(), outputArea.height(), QImage::Format_RGB32);
    bumpmap.fill(0);

    QRgb** bumpmapByteLines = new QRgb*[bumpmap.height()];

    for (int yIndex = 0; yIndex < bumpmap.height(); yIndex++)
        bumpmapByteLines[yIndex] = (QRgb *) bumpmap.scanLine(yIndex);

    qDebug("Tiempo de total preparacion: %d ms", timer.restart());

    // Tiles need to overlap in 2 pixels, to prevent seams between tiles
    // because only the inner pixels of each tile will be rendered
    const int TILE_OFFSET = 2;
    const int TILE_WIDTH = 100;
    const int TILE_HEIGHT = 100;
    const int TILE_WIDTH_MINUS_1 = TILE_WIDTH - 1;
    const int TILE_HEIGHT_MINUS_1 = TILE_HEIGHT - 1;
    const int COLS_OF_TILES = inputArea.width() / TILE_WIDTH;
    const int ROWS_OF_TILES = inputArea.height() / TILE_HEIGHT;
    const int STRAY_COL_WIDTH = (TILE_OFFSET * COLS_OF_TILES) + inputArea.width() % TILE_WIDTH;
    const int STRAY_ROW_HEIGHT = (TILE_OFFSET * ROWS_OF_TILES) + inputArea.height() % TILE_HEIGHT;
    const int X_READ_OFFSET = applyRect.topLeft().x();
    const int Y_READ_OFFSET = applyRect.topLeft().y();
    const int OUTPUT_OFFSET = 1;

    QVector<quint8*> tileChannels;
    PhongPixelProcessor tileRenderer(config);

    //======================================
    //===============RENDER=================
    //======================================

    for (int col = 0; col < COLS_OF_TILES; col++) {
        for (int row = 0; row < ROWS_OF_TILES; row++) {
            // ^^^  Foreach tile

            // See TILE_OFFSET for explanation
            tileLimits.setX( 0 + (TILE_WIDTH - TILE_OFFSET) * col);
            tileLimits.setY( 0 + (TILE_HEIGHT - TILE_OFFSET) * row);
            tileLimits.setWidth( TILE_WIDTH );
            tileLimits.setHeight( TILE_HEIGHT );

            tileChannels = device->readPlanarBytes(tileLimits.x() + X_READ_OFFSET,
                                                tileLimits.y() + Y_READ_OFFSET,
                                                tileLimits.width(),
                                                tileLimits.height()
                                                );
            quint8* tileHeightmap = tileChannels.data()[m_heightChannel->index()];
            tileRenderer.heightmap = tileHeightmap;

            for (int x = 1; x < TILE_WIDTH_MINUS_1; x++) {
                for (int y = 1; y < TILE_HEIGHT_MINUS_1; y++) {
                    // ^^^ Foreach INNER pixel in tile

                    posup   = (y + 1) * tileLimits.width() + x;
                    posdown = (y - 1) * tileLimits.width() + x;
                    posleft  = y * tileLimits.width() + x - 1;
                    posright = y * tileLimits.width() + x + 1;

                    bumpmapByteLines[y + tileLimits.y() - OUTPUT_OFFSET][x + tileLimits.x() - OUTPUT_OFFSET] =
                    //tileRenderer.reallyFastIlluminatePixel(posup, posdown, posleft, posright);
                    tileRenderer.testingSpeedIlluminatePixel(posup, posdown, posleft, posright);
                }
            }
        }
    }

    // Fill the stray [||] column, past the last square tiles in the row
    if (STRAY_COL_WIDTH > TILE_OFFSET) {
        const int STRAY_COL_WIDTH_MINUS_1 = STRAY_COL_WIDTH - 1;
        tileLimits.setX( 0 + (TILE_WIDTH - TILE_OFFSET) * COLS_OF_TILES );
        for (int row = 0; row < ROWS_OF_TILES; row++) {
            // See TILE_OFFSET for explanation
            tileLimits.setY( 0 + (TILE_HEIGHT - TILE_OFFSET) * row );
            tileLimits.setWidth( STRAY_COL_WIDTH );
            tileLimits.setHeight( TILE_HEIGHT );

            tileChannels = device->readPlanarBytes(tileLimits.x() + X_READ_OFFSET,
                                                tileLimits.y() + Y_READ_OFFSET,
                                                tileLimits.width(),
                                                tileLimits.height()
                                                );
            quint8* tileHeightmap = tileChannels.data()[m_heightChannel->index()];
            tileRenderer.heightmap = tileHeightmap;

            for (int x = 1; x < STRAY_COL_WIDTH_MINUS_1; x++) {
                for (int y = 1; y < TILE_HEIGHT_MINUS_1; y++) {
                    // ^^^ Foreach INNER pixel in tile

                    posup   = (y + 1) * tileLimits.width() + x;
                    posdown = (y - 1) * tileLimits.width() + x;
                    posleft  = y * tileLimits.width() + x - 1;
                    posright = y * tileLimits.width() + x + 1;

                    bumpmapByteLines[y + tileLimits.y() - OUTPUT_OFFSET][x + tileLimits.x() - OUTPUT_OFFSET] =
                    //tileRenderer.reallyFastIlluminatePixel(posup, posdown, posleft, posright);
                    tileRenderer.testingSpeedIlluminatePixel(posup, posdown, posleft, posright);
                }
            }
        }
    }

    // Fill the stray [=] row, past the last square tiles in the columns
    if (STRAY_COL_WIDTH > TILE_OFFSET) {
        const int STRAY_ROW_HEIGHT_MINUS_1 = STRAY_ROW_HEIGHT - 1;
        tileLimits.setY( 0 + (TILE_HEIGHT - TILE_OFFSET) * ROWS_OF_TILES );
        for (int col = 0; col < COLS_OF_TILES; col++) {
            // See TILE_OFFSET for explanation
            tileLimits.setX( 0 + (TILE_WIDTH - TILE_OFFSET) * col);
            tileLimits.setWidth( TILE_WIDTH );
            tileLimits.setHeight( STRAY_ROW_HEIGHT );

            tileChannels = device->readPlanarBytes(tileLimits.x() + X_READ_OFFSET,
                                                tileLimits.y() + Y_READ_OFFSET,
                                                tileLimits.width(),
                                                tileLimits.height()
                                                );
            quint8* tileHeightmap = tileChannels.data()[m_heightChannel->index()];
            tileRenderer.heightmap = tileHeightmap;

            for (int x = 1; x < TILE_HEIGHT_MINUS_1; x++) {
                for (int y = 1; y < STRAY_ROW_HEIGHT_MINUS_1; y++) {
                    // ^^^ Foreach INNER pixel in tile

                    posup   = (y + 1) * tileLimits.width() + x;
                    posdown = (y - 1) * tileLimits.width() + x;
                    posleft  = y * tileLimits.width() + x - 1;
                    posright = y * tileLimits.width() + x + 1;

                    bumpmapByteLines[y + tileLimits.y() - OUTPUT_OFFSET][x + tileLimits.x() - OUTPUT_OFFSET] =
                    //tileRenderer.reallyFastIlluminatePixel(posup, posdown, posleft, posright);
                    tileRenderer.testingSpeedIlluminatePixel(posup, posdown, posleft, posright);
                }
            }
        }
    }

    // Fill the stray [.] corner, past the last square column and row
    if ((STRAY_COL_WIDTH > TILE_OFFSET) && (STRAY_ROW_HEIGHT > TILE_OFFSET)) {
        const int STRAY_ROW_HEIGHT_MINUS_1 = STRAY_ROW_HEIGHT - 1;
        const int STRAY_COL_WIDTH_MINUS_1 = STRAY_COL_WIDTH - 1;
        // See TILE_OFFSET for explanation
        tileLimits.setX( 0 + (TILE_WIDTH - TILE_OFFSET) * COLS_OF_TILES );
        tileLimits.setY( 0 + (TILE_HEIGHT - TILE_OFFSET) * ROWS_OF_TILES );
        tileLimits.setWidth( STRAY_COL_WIDTH );
        tileLimits.setHeight( STRAY_ROW_HEIGHT );

        tileChannels = device->readPlanarBytes(tileLimits.x() + X_READ_OFFSET,
                                            tileLimits.y() + Y_READ_OFFSET,
                                            tileLimits.width(),
                                            tileLimits.height()
                                            );
        quint8* tileHeightmap = tileChannels.data()[m_heightChannel->index()];
        tileRenderer.heightmap = tileHeightmap;

        for (int x = 1; x < STRAY_COL_WIDTH_MINUS_1; x++) {
            for (int y = 1; y < STRAY_ROW_HEIGHT_MINUS_1; y++) {
                // ^^^ Foreach INNER pixel in tile

                posup   = (y + 1) * tileLimits.width() + x;
                posdown = (y - 1) * tileLimits.width() + x;
                posleft  = y * tileLimits.width() + x - 1;
                posright = y * tileLimits.width() + x + 1;

                bumpmapByteLines[y + tileLimits.y() - OUTPUT_OFFSET][x + tileLimits.x() - OUTPUT_OFFSET] =
                //tileRenderer.reallyFastIlluminatePixel(posup, posdown, posleft, posright);
                tileRenderer.testingSpeedIlluminatePixel(posup, posdown, posleft, posright);
            }
        }
    }


    qDebug("Tiempo de calculo: %d ms", timer.restart());
    /*
    KisPainter leHack(dstInfo.paintDevice());
    KisPaintDeviceSP sneaky = new KisPaintDevice(dstInfo.paintDevice()->colorSpace());
    sneaky->convertFromQImage(bumpmap, "");
    leHack.bitBlt(dstInfo.topLeft(), sneaky, sneaky->exactBounds());
    */
    //qDebug() << bumpmap.size();
    device->convertFromQImage(bumpmap, "", applyRect.topLeft().x(), applyRect.topLeft().y());
    qDebug("Tiempo deconversion: %d ms", timer.elapsed());

    delete [] bumpmapByteLines;
}



KisFilterPhongBumpmap::KisFilterPhongBumpmap()
                      : KisFilter(KoID("phongbumpmap", i18n("PhongBumpmap")), KisFilter::categoryMap(), i18n("&PhongBumpmap..."))
{
    setColorSpaceIndependence(TO_LAB16);
    setSupportsPainting(true);
    setSupportsIncrementalPainting(true);
}


KisFilterConfiguration* KisFilterPhongBumpmap::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id(), 0);
    return config;
}

QRect KisFilterPhongBumpmap::neededRect(const QRect &rect, const KisFilterConfiguration* /*config*/) const
{
    return rect.adjusted(-2, -2, 2, 2);
}

QRect KisFilterPhongBumpmap::changedRect(const QRect &rect, const KisFilterConfiguration* /*config*/) const
{
    return rect.adjusted(-2, -2, 2, 2);
}






KisConfigWidget * KisFilterPhongBumpmap::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    KisPhongBumpmapConfigWidget * w = new KisPhongBumpmapConfigWidget(dev, image, parent);

    return w;
}










KisPhongBumpmapConfigWidget::KisPhongBumpmapConfigWidget(const KisPaintDeviceSP dev, const KisImageWSP image, QWidget * parent, Qt::WFlags f)
                            : KisConfigWidget(parent, f)
                            , m_device(dev)
                            , m_image(image)
{
    Q_ASSERT(m_device);
    m_page = new PhongBumpmapWidget(this);

    connect(m_page->azimuthDial1, SIGNAL(valueChanged(int)), m_page->azimuthSpinBox1, SLOT(setValue(int)));
    connect(m_page->azimuthDial2, SIGNAL(valueChanged(int)), m_page->azimuthSpinBox2, SLOT(setValue(int)));
    connect(m_page->azimuthDial3, SIGNAL(valueChanged(int)), m_page->azimuthSpinBox3, SLOT(setValue(int)));
    connect(m_page->azimuthDial4, SIGNAL(valueChanged(int)), m_page->azimuthSpinBox4, SLOT(setValue(int)));
    connect(m_page->azimuthSpinBox1, SIGNAL(valueChanged(int)), m_page->azimuthDial1, SLOT(setValue(int)));
    connect(m_page->azimuthSpinBox2, SIGNAL(valueChanged(int)), m_page->azimuthDial2, SLOT(setValue(int)));
    connect(m_page->azimuthSpinBox3, SIGNAL(valueChanged(int)), m_page->azimuthDial3, SLOT(setValue(int)));
    connect(m_page->azimuthSpinBox4, SIGNAL(valueChanged(int)), m_page->azimuthDial4, SLOT(setValue(int)));

    connect(m_page->diffuseReflectivityCheckBox, SIGNAL(toggled(bool)),
            m_page->diffuseReflectivityKisDoubleSliderSpinBox, SLOT(setEnabled(bool)));
    connect(m_page->specularReflectivityCheckBox, SIGNAL(toggled(bool)),
            m_page->specularReflectivityKisDoubleSliderSpinBox, SLOT(setEnabled(bool)));
    connect(m_page->specularReflectivityCheckBox, SIGNAL(toggled(bool)),
            m_page->shinynessExponentKisSliderSpinBox, SLOT(setEnabled(bool)));
    connect(m_page->specularReflectivityCheckBox, SIGNAL(toggled(bool)),
            m_page->shinynessExponentLabel, SLOT(setEnabled(bool)));

    QVBoxLayout * l = new QVBoxLayout(this);
    Q_CHECK_PTR(l);

    l->addWidget(m_page);

    /* fill in the channel chooser */
    QList<KoChannelInfo *> channels = m_device->colorSpace()->channels();
    for (quint8 ch = 0; ch < m_device->colorSpace()->colorChannelCount(); ch++)
        m_page->heightChannelComboBox->addItem(channels.at(ch)->name());


}

void KisPhongBumpmapConfigWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    QVariant tempcolor;
    m_page->ambientReflectivityKisDoubleSliderSpinBox->setValue( config->getDouble(PHONG_AMBIENT_REFLECTIVITY) );
    m_page->diffuseReflectivityKisDoubleSliderSpinBox->setValue( config->getDouble(PHONG_DIFFUSE_REFLECTIVITY) );
    m_page->specularReflectivityKisDoubleSliderSpinBox->setValue( config->getDouble(PHONG_SPECULAR_REFLECTIVITY) );
    m_page->shinynessExponentKisSliderSpinBox->setValue( config->getInt(PHONG_SHINYNESS_EXPONENT) );
    m_page->diffuseReflectivityCheckBox->setChecked( config->getBool(PHONG_DIFFUSE_REFLECTIVITY_IS_ENABLED) );
    m_page->specularReflectivityCheckBox->setChecked( config->getBool(PHONG_SPECULAR_REFLECTIVITY_IS_ENABLED) );
    // Indexes are off by 1 simply because arrays start at 0 and the GUI naming scheme started at 1
    m_page->lightSourceGroupBox1->setChecked( config->getBool(PHONG_ILLUMINANT_IS_ENABLED[0]) );
    m_page->lightSourceGroupBox2->setChecked( config->getBool(PHONG_ILLUMINANT_IS_ENABLED[1]) );
    m_page->lightSourceGroupBox3->setChecked( config->getBool(PHONG_ILLUMINANT_IS_ENABLED[2]) );
    m_page->lightSourceGroupBox4->setChecked( config->getBool(PHONG_ILLUMINANT_IS_ENABLED[3]) );
    config->getProperty(PHONG_ILLUMINANT_COLOR[0], tempcolor);
    m_page->lightKColorCombo1->setColor(tempcolor.value<QColor>());
    config->getProperty(PHONG_ILLUMINANT_COLOR[1], tempcolor);
    m_page->lightKColorCombo2->setColor(tempcolor.value<QColor>());
    config->getProperty(PHONG_ILLUMINANT_COLOR[2], tempcolor);
    m_page->lightKColorCombo3->setColor(tempcolor.value<QColor>());
    config->getProperty(PHONG_ILLUMINANT_COLOR[3], tempcolor);
    m_page->lightKColorCombo4->setColor(tempcolor.value<QColor>());
    m_page->azimuthSpinBox1->setValue( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[0]) );
    m_page->azimuthSpinBox2->setValue( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[1]) );
    m_page->azimuthSpinBox3->setValue( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[2]) );
    m_page->azimuthSpinBox4->setValue( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[3]) );
    m_page->inclinationSpinBox1->setValue( config->getDouble(PHONG_ILLUMINANT_INCLINATION[0]) );
    m_page->inclinationSpinBox2->setValue( config->getDouble(PHONG_ILLUMINANT_INCLINATION[1]) );
    m_page->inclinationSpinBox3->setValue( config->getDouble(PHONG_ILLUMINANT_INCLINATION[2]) );
    m_page->inclinationSpinBox4->setValue( config->getDouble(PHONG_ILLUMINANT_INCLINATION[3]) );

    if (!config) return;
}

KisPropertiesConfiguration* KisPhongBumpmapConfigWidget::configuration() const
{
    KisFilterConfiguration * config = new KisFilterConfiguration("phongbumpmap", 2);
    config->setProperty(PHONG_HEIGHT_CHANNEL, m_page->heightChannelComboBox->currentText());
    config->setProperty(PHONG_AMBIENT_REFLECTIVITY, m_page->ambientReflectivityKisDoubleSliderSpinBox->value());
    config->setProperty(PHONG_DIFFUSE_REFLECTIVITY, m_page->diffuseReflectivityKisDoubleSliderSpinBox->value());
    config->setProperty(PHONG_SPECULAR_REFLECTIVITY, m_page->specularReflectivityKisDoubleSliderSpinBox->value());
    config->setProperty(PHONG_SHINYNESS_EXPONENT, m_page->shinynessExponentKisSliderSpinBox->value());
    config->setProperty(PHONG_DIFFUSE_REFLECTIVITY_IS_ENABLED, m_page->diffuseReflectivityCheckBox->isChecked());
    config->setProperty(PHONG_SPECULAR_REFLECTIVITY_IS_ENABLED, m_page->specularReflectivityCheckBox->isChecked());
    //config->setProperty(PHONG_SHINYNESS_EXPONENT_IS_ENABLED, m_page->specularReflectivityCheckBox->isChecked());
    // Indexes are off by 1 simply because arrays start at 0 and the GUI naming scheme started at 1
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[0], m_page->lightSourceGroupBox1->isChecked());
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[1], m_page->lightSourceGroupBox2->isChecked());
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[2], m_page->lightSourceGroupBox3->isChecked());
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[3], m_page->lightSourceGroupBox4->isChecked());
    config->setProperty(PHONG_ILLUMINANT_COLOR[0], m_page->lightKColorCombo1->color());
    config->setProperty(PHONG_ILLUMINANT_COLOR[1], m_page->lightKColorCombo2->color());
    config->setProperty(PHONG_ILLUMINANT_COLOR[2], m_page->lightKColorCombo3->color());
    config->setProperty(PHONG_ILLUMINANT_COLOR[3], m_page->lightKColorCombo4->color());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[0], m_page->azimuthSpinBox1->value());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[1], m_page->azimuthSpinBox2->value());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[2], m_page->azimuthSpinBox3->value());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[3], m_page->azimuthSpinBox4->value());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[0], m_page->inclinationSpinBox1->value());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[1], m_page->inclinationSpinBox2->value());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[2], m_page->inclinationSpinBox3->value());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[3], m_page->inclinationSpinBox4->value());

    // Read configuration
    QMap<QString, QVariant> rofl = QMap<QString, QVariant>(config->getProperties());

    QMap<QString, QVariant>::const_iterator i;
    for (i = rofl.constBegin(); i != rofl.constEnd(); ++i)
        qDebug() << i.key() << ":" << i.value();

    return config;
}

#include "phongbumpmap.moc"

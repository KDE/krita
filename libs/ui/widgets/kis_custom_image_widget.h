/* This file is part of the Calligra project
 * SPDX-FileCopyrightText: 2005 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CUSTOM_IMAGE_WIDGET_H
#define KIS_CUSTOM_IMAGE_WIDGET_H

#include "kis_global.h"
#include "KoUnit.h"
#include "kis_properties_configuration.h"
#include "KisOpenPane.h"

#include <ui_wdgnewimage.h>

class KisDocument;
class KisDocument;

enum CustomImageWidgetType { CUSTOM_DOCUMENT, NEW_IMG_FROM_CB };

class WdgNewImage : public QWidget, public Ui::WdgNewImage
{
    Q_OBJECT

public:
    WdgNewImage(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

/**
 * The 'Custom Document' widget in the Krita startup widget.
 * This class embeds the image size and colorspace to allow the user to select the image properties
 * for a new empty image document.
 */
class KisCustomImageWidget : public WdgNewImage
{
    Q_OBJECT
public:
    /**
     * Constructor. Please note that this class is being used/created by KisDoc.
     * @param parent the parent widget
     * @param defWidth The defined width
     * @param defHeight The defined height
     * @param resolution The image resolution
     * @param defColorModel The defined color model
     * @param defColorDepth The defined color depth
     * @param defColorProfile The defined color profile
     * @param imageName the document that wants to be altered
     */
    KisCustomImageWidget(QWidget *parent, qint32 defWidth, qint32 defHeight, double resolution, const QString & defColorModel, const QString & defColorDepth, const QString & defColorProfile, const QString & imageName);
    ~KisCustomImageWidget() override;
    
private Q_SLOTS:
    void widthUnitChanged(int index);
    void widthChanged(double value);
    void heightUnitChanged(int index);
    void heightChanged(double value);
    void resolutionChanged(double value);
    void clipboardDataChanged();
    void predefinedClicked(int index);
    void saveAsPredefined();
    void setLandscape();
    void setPortrait();
    void switchWidthHeight();
    void createImage();
    void switchPortraitLandscape();
    void changeDocumentInfoLabel();
    void resolutionUnitChanged();

protected:
    
    KisDocument *createNewImage();
    
    /// Set the number of layers that will be created
    void setNumberOfLayers(int layers);

    KisOpenPane *m_openPane;
private:
    
    double m_width, m_height;
    quint8 backgroundOpacity() const;
    void setBackgroundOpacity(quint8 value);

    void fillPredefined();
    void showEvent(QShowEvent *) override;
    
    KoUnit m_widthUnit, m_heightUnit;
    QList<KisPropertiesConfigurationSP> m_predefined;

};

#endif

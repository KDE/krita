/****************************************************************************
** Form interface generated from reading ui file './kis_custom_convolution_filter_configuration_base_widget.ui'
**
** Created: lun sep 27 15:20:14 2004
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef KISCUSTOMCONVOLUTIONFILTERCONFIGURATIONBASEWIDGET_H
#define KISCUSTOMCONVOLUTIONFILTERCONFIGURATIONBASEWIDGET_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class KisMatrixWidget;
class QLabel;
class QSpinBox;

class KisCustomConvolutionFilterConfigurationBaseWidget : public QWidget
{
    Q_OBJECT

public:
    KisCustomConvolutionFilterConfigurationBaseWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~KisCustomConvolutionFilterConfigurationBaseWidget();

    KisMatrixWidget* matrixWidget;
    QLabel* textLabelFactor;
    QSpinBox* spinBoxFactor;
    QLabel* textLabelOffset;
    QSpinBox* spinBoxOffset;

protected:
    QHBoxLayout* KisCustomConvolutionFilterConfigurationBaseWidgetLayout;
    QSpacerItem* spacer5;
    QVBoxLayout* layout5;
    QSpacerItem* spacer24;
    QHBoxLayout* layout4;
    QSpacerItem* spacer1;
    QHBoxLayout* layout3;
    QSpacerItem* spacer2;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;

};

#endif // KISCUSTOMCONVOLUTIONFILTERCONFIGURATIONBASEWIDGET_H

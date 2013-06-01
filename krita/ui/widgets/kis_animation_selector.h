#ifndef KIS_ANIMATION_SELECTOR_H
#define KIS_ANIMATION_SELECTOR_H

#include <QWidget>
#include "kis_global.h"
#include "KoUnit.h"
#include "kis_properties_configuration.h"
#include "ui_wdganimationselector.h"

class KisDoc2;
class KoID;

class WdgAnimationSelector : public QWidget, public Ui::WdgAnimationSelector{
    Q_OBJECT
public:
    WdgAnimationSelector(QWidget* parent) : QWidget(parent){
        setupUi(this);
    }
};

class KisAnimationSelector : public WdgAnimationSelector
{
    Q_OBJECT
public:
    KisAnimationSelector(QWidget* parent, KisDoc2 *document, qint32 defWidth, qint32 defHeight, double resolution, const QString & defColorModel, const QString &defColorDepth, const QString &defColorProfile, const QString&animationName);
    virtual ~KisAnimationSelector();

signals:
    void documentSelected();

private slots:
    void createAnimation();
    void resolutionChanged(double value);
    void widthUnitChanged(int index);
    void heightUnitChanged(int index);
    void widthChanged(double value);
    void heightChanged(double value);

private:
    quint8 backgroundOpacity();
    KisDoc2 *m_document;
    double m_width, m_height;
    KoUnit m_widthUnit, m_heightUnit;
    QList<KisPropertiesConfiguration*> m_predefined;
};

#endif // KIS_ANIMATION_SELECTOR_H

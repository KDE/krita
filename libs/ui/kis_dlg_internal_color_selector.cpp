#include <QHBoxLayout>
#include <QDebug>

#include "kis_dlg_internal_color_selector.h"
#include "kis_screen_color_picker.h"

KisDlgInternalColorSelector::KisDlgInternalColorSelector(QWidget* parent, KoColor color, Config config, const QString &caption, const KoColorDisplayRendererInterface *displayRenderer)
    : KisBasicInternalColorSelector(parent, color, config, caption, displayRenderer)
{
    m_screenColorPicker = new KisScreenColorPicker(this);
    m_ui->screenColorPickerWidget->setLayout(new QHBoxLayout(m_ui->screenColorPickerWidget));
    m_ui->screenColorPickerWidget->layout()->addWidget(m_screenColorPicker);
    qDebug() << "dlg setup";
    if (config.screenColorPicker) {
        connect(m_screenColorPicker, SIGNAL(sigNewColorPicked(KoColor)),this, SLOT(slotColorUpdated(KoColor)));
        qDebug() << "dlg setup connected";
    } else {
        m_ui->screenColorPickerWidget->hide();
    }
}


void KisDlgInternalColorSelector::updateAllElements(QObject *source)
{
    KisBasicInternalColorSelector::updateAllElements(source);
    m_screenColorPicker->updateIcons();
}

KoColor KisDlgInternalColorSelector::getModalColorDialog(const KoColor color, QWidget* parent, QString caption)
{
    Config config = Config();
    KisDlgInternalColorSelector dialog(parent, color, config, caption);
    dialog.setPreviousColor(color);
    dialog.exec();
    return dialog.getCurrentColor();
}

#include "kis_dlg_send_telemetry.h"

WdgTelemetry::WdgTelemetry(QWidget* parent)
{
}

KisDlgSendTelemtry::KisDlgSendTelemtry(KisViewManager* view)
    : KoDialog(view->mainWindow())
{
    m_widget.reset(new WdgTelemetry(this));

    setMainWidget(m_widget.data());
    resize(m_widget->sizeHint());
}

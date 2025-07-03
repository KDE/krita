#ifndef _KIS_SELECTION_ASSISTANTS_DECORATION_H_
#define _KIS_SELECTION_ASSISTANTS_DECORATION_H_

#include <QPointF>
#include <QColor>

#include "KoPointerEvent.h"
#include "KoSnapGuide.h"
#include "kis_icon_utils.h"
#include "canvas/kis_canvas_decoration.h"
#include "kis_painting_assistant.h"
#include <kritaui_export.h>

class KisView;

class KisSelectionAssistantsDecoration;
typedef KisSharedPtr<KisSelectionAssistantsDecoration> KisSelectionAssistantsDecorationSP;

/**
 * placeholder
 * KisSelectionAssistantsDecoration draws the assistants stored in the document on
 * the canvas.
 * In the application flow, each canvas holds one of these classes to manage the assistants
 * There is an assistants manager, but that is higher up in the flow and makes sure each view gets one of these
 * Since this is off the canvas level, the decoration can be seen across all tools. The contents from here will be in
 * front of the kis_assistant_tool, which hold and displays the editor controls.
 *
 * Many of the events this receives such as adding and removing assistants comes from kis_assistant_tool
 */
class KRITAUI_EXPORT KisSelectionAssistantsDecoration
{
    //Q_OBJECT
public:
    KisSelectionAssistantsDecoration();
    ~KisSelectionAssistantsDecoration();
    void drawDecoration(QPainter& gc, const KisCoordinatesConverter *converter);

private:
    struct Private;
    Private* const d;
};

#endif

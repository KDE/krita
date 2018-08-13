#include <kundo2command.h>
#include <kundo2magicstring.h>

#include "KisChangePaletteCommand.h"

const char KisChangePaletteCommand::MagicString[] = "Edit palette";

KisChangePaletteCommand::KisChangePaletteCommand()
    : KUndo2Command(kundo2_i18n(MagicString))
{

}

KisChangePaletteCommand::~KisChangePaletteCommand()
{ }

int KisChangePaletteCommand::id() const
{
    return KisCommandUtils::ChangePaletteId;
}

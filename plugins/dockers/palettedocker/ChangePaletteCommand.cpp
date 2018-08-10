#include <kundo2command.h>
#include <kundo2magicstring.h>

#include "ChangePaletteCommand.h"

const char ChangePaletteCommand::MagicString[] = "Edit palette";

ChangePaletteCommand::ChangePaletteCommand()
    : KUndo2Command(kundo2_i18n(MagicString))
{

}

ChangePaletteCommand::~ChangePaletteCommand()
{ }

int ChangePaletteCommand::id() const
{
    return KisCommandUtils::ChangePaletteId;
}

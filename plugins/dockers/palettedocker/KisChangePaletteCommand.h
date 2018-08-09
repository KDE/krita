#ifndef KISCHANGEPALETTECOMMAND_H
#define KISCHANGEPALETTECOMMAND_H

#include <QString>

#include <kundo2command.h>
#include <kis_command_ids.h>

class KisChangePaletteCommand : public KUndo2Command
{
public:
    static const char MagicString[];
public:
    KisChangePaletteCommand();
    ~KisChangePaletteCommand() override;
    void undo() override { }
    void redo() override { }
    int id() const override;
};

#endif // KISCHANGEPALETTECOMMAND_H

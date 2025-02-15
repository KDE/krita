'''
  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later

  This python3 script generates the KoUnicodeBlockData.cpp file from the
  unicode character database "Blocks.txt" file. This is so that we don't have to
  manually add new entries everytime a new version of unicode is released.
'''
import urllib.request

outPath = "KoUnicodeBlockData.cpp"
outFile = open(outPath,'w');

blocks = []
with urllib.request.urlopen('https://www.unicode.org/Public/16.0.0/ucd/Blocks.txt') as f:
    blocksFile = f.read().decode('utf-8').split("\n")
    for line in blocksFile:
        if line.startswith("#") or len(line) == 0:
            continue
        else:
            block = {}
            block["name"] = line.split(";")[-1].strip()
            codes = line.split(";")[0];
            block["start"] = codes.split("..")[0].strip()
            block["end"] = codes.split("..")[-1].strip()
            blocks.append(block)

print(blocks)

outFile.writelines(
    [ "/*"
    , "\n *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>"
    , "\n *"
    , "\n *  SPDX-License-Identifier: GPL-2.0-or-later"
    , "\n *  Generated from https://www.unicode.org/Public/16.0.0/ucd/Blocks.txt"
    , "\n */"])

outFile.writelines([
    "\n\n#include \"KoUnicodeBlockData.h\""
        , "\n#include <QVector>"
        ])
outFile.write("\n")
outFile.writelines(
    ["\nstruct Q_DECL_HIDDEN KoUnicodeBlockDataFactory::Private",
     "\n{",
     "\n    QVector<KoUnicodeBlockData> blockMap;",
     "\n};"])
outFile.write("\n")
outFile.writelines(["\nKoUnicodeBlockDataFactory::KoUnicodeBlockDataFactory()", "\n    : d(new Private)", "\n{",])

### Start writing the unicode blocks

outFile.write("\n")
for block in blocks:
    outFile.writelines(["\n    d->blockMap.append(KoUnicodeBlockData(i18nc(\"@title\", \"" + block["name"] + "\"), QChar(0x" + block["start"] + "), QChar(0x" + block["end"] + ")));"])

outFile.writelines("\n}")

# Destructor
outFile.write("\n")
outFile.writelines(["\nKoUnicodeBlockDataFactory::~KoUnicodeBlockDataFactory()", "\n{","\n}"])

outFile.write("\n")
outFile.writelines(["\nKoUnicodeBlockData KoUnicodeBlockDataFactory::blockForUCS(QChar codepoint)", "\n{"])
outFile.writelines(["\n    for (int i = 0; i < d->blockMap.size(); i++) {","\n        KoUnicodeBlockData block = d->blockMap.at(i);","\n        if (block.match(codepoint)) {","\n                return block;","\n        }","\n    }\n    return noBlock();","\n}"])

outFile.close()

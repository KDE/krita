#include "sai.hpp"

#include <algorithm>
#include <stack>

namespace sai
{
    /// Internal structures:
#pragma pack(push, 1)
    struct LayerTableEntry
    {
        uint32_t Identifier;
        enum class LayerType : uint16_t
        {
            RootLayer = 0x00, // Parent Canvas layer object
            Layer = 0x03, // Regular Layer
            Unknown4 = 0x4, // Unknown
            Linework = 0x05, // Vector Linework Layer
            Mask = 0x06, // Masks applied to any layer object
            Unknown7 = 0x07, //Unknown
            Set = 0x08//Layer Folder
        } Type;
        // These all get added and sent as a windows message 0x80CA for some reason
        uint16_t Unknown;
    };

    struct LayerHeader
    {
        enum class Type : uint32_t
        {
            RootLayer = 0x00, // Parent Canvas layer object
            Layer = 0x03, // Regular Layer
            Unknown4 = 0x4, // Unknown
            Linework = 0x05, // Vector Linework Layer
            Mask = 0x06, // Masks applied to any layer object
            Unknown7 = 0x07, //Unknown
            Set = 0x08//Layer Folder
        } Type;
        uint32_t Identifier;
        struct LayerBounds
        {
            int32_t X; // (X / 32) * 32
            int32_t Y; // (Y / 32) * 32
            uint32_t Width; // Width - 31
            uint32_t Height; // Height - 31
        } Bounds;
        uint32_t Unknown;
        uint8_t Opacity;
        uint8_t Visible;
        uint8_t PreserveOpacity;
        uint8_t Clipping;
        uint8_t Unknown4;
        char Blending[sizeof(uint32_t)];
    };
#pragma pack(pop)

    /// File Entry
    VirtualFileEntry::VirtualFileEntry()
        :
        Position(0),
        FileSystem(nullptr),
        Data()
    {
    }

    VirtualFileEntry::VirtualFileEntry(VirtualFileSystem &FileSystem)
        :
        Position(0),
        FileSystem(&FileSystem),
        Data()
    {
    }

    VirtualFileEntry::~VirtualFileEntry()
    {
    }

    uint32_t VirtualFileEntry::GetFlags() const
    {
        return Data.Flags;
    }

    const char * VirtualFileEntry::GetName() const
    {
        return Data.Name;
    }

    VirtualFileEntry::EntryType VirtualFileEntry::GetType() const
    {
        return Data.Type;
    }

    size_t VirtualFileEntry::GetBlock() const
    {
        return static_cast<size_t>(Data.Block);
    }

    size_t VirtualFileEntry::GetSize() const
    {
        return static_cast<size_t>(Data.Size);
    }

    time_t VirtualFileEntry::GetTimeStamp() const
    {
        return Data.TimeStamp / 10000000ULL - 11644473600ULL;
    }

    size_t VirtualFileEntry::Tell() const
    {
        return Position;
    }

    void VirtualFileEntry::Seek(size_t Offset)
    {
        Position = Offset;
    }

    bool VirtualFileEntry::Read(void *Destination, size_t Size)
    {
        if( FileSystem )
        {
            FileSystem->Read(
                (static_cast<size_t>(Data.Block) * FileSystemBlock::BlockSize)
                + Position,
                Size,
                Destination);
            Position += Size;
            return true;
        }
        return false;
    }

    /// File System
    VirtualFileSystem::VirtualFileSystem()
        :
        CacheTable(nullptr),
        CacheBuffer(nullptr)
    {
        CacheTable = std::make_unique<FileSystemBlock>();
        CacheBuffer = std::make_unique<FileSystemBlock>();
    }

    VirtualFileSystem::~VirtualFileSystem()
    {
        if( FileStream )
        {
            FileStream.close();
        }
    }

    bool VirtualFileSystem::Mount(const char *FileName)
    {
        if( FileStream )
        {
            FileStream.close();
        }

        FileStream.open(FileName, std::ios::binary | std::ios::ate);

        if( FileStream )
        {
            std::ifstream::pos_type FileSize = FileStream.tellg();

            if( FileSize & 0x1FF )
            {
                // File size is not Block-aligned
                FileStream.close();
                return false;
            }

            BlockCount = static_cast<size_t>(FileSize) / FileSystemBlock::BlockSize;

            // Verify all Blocks
            for( size_t i = 0; i < BlockCount; i++ )
            {
                GetBlock(i, CacheBuffer.get());
                if( i & 0x1FF ) // Block is data
                {
                    if( CacheTable->TableEntries[i & 0x1FF].Checksum != CacheBuffer->Checksum(false) )
                    {
                        // Checksum mismatch. Data invalid
                        FileStream.close();
                        return false;
                    }
                }
                else // Block is a table
                {
                    if( CacheTable->TableEntries[0].Checksum != CacheTable->Checksum(true) )
                    {
                        // Checksum mismatch. Table invalid
                        FileStream.close();
                        return false;
                    }
                }
            }
            return true;
        }
        return false;
    }

    size_t VirtualFileSystem::GetBlockCount() const
    {
        return BlockCount;
    }

    size_t VirtualFileSystem::GetSize() const
    {
        return GetBlockCount() * FileSystemBlock::BlockSize;
    }

    bool VirtualFileSystem::GetEntry(const char *Path, FileEntry &Entry)
    {
        if( FileStream )
        {
            GetBlock(2, CacheBuffer.get());

            std::string CurPath(Path);

            const char* CurToken = std::strtok(&CurPath[0], "./");

            size_t CurEntry = 0;
            while( CurEntry < 64 && CacheBuffer->FATEntries[CurEntry].Flags )
            {
                if( std::strcmp(CurToken, CacheBuffer->FATEntries[CurEntry].Name) == 0 )
                {
                    if( (CurToken = std::strtok(nullptr, "./")) == nullptr ) // No more tokens to process, done
                    {
                        Entry.Data = CacheBuffer->FATEntries[CurEntry];
                        Entry.FileSystem = this;
                        return true;
                    }

                    if( CacheBuffer->FATEntries[CurEntry].Type != VirtualFileEntry::EntryType::Folder )
                    {
                        // Entry is not a folder, cant go further
                        return false;
                    }
                    GetBlock(
                        CacheBuffer->FATEntries[CurEntry].Block,
                        CacheBuffer.get()
                    );
                    CurEntry = 0;
                    continue;
                }
                CurEntry++;
            }
        }
        return false;
    }

    bool VirtualFileSystem::Read(size_t Offset, size_t Size, void *Destination)
    {
        if( FileStream )
        {
            uint8_t *WritePoint = reinterpret_cast<uint8_t*>(Destination);

            while( Size )
            {
                size_t CurBlock = Offset / FileSystemBlock::BlockSize; // Nearest Block Offset
                size_t CurBlockOffset = Offset % FileSystemBlock::BlockSize; // Offset within Block
                size_t CurBlockSize = std::min<size_t>(
                    Size,
                    FileSystemBlock::BlockSize - CurBlockOffset
                    ); // Size within Block

                // Current Block to read from
                GetBlock(
                    CurBlock,
                    CacheBuffer.get()
                );

                memcpy(WritePoint, CacheBuffer->u8 + CurBlockOffset, CurBlockSize);

                Size -= CurBlockSize;
                WritePoint += CurBlockSize;
                Offset += CurBlockSize;
                CurBlock++;
            }
            return true;
        }
        return false;
    }

    void VirtualFileSystem::IterateFileSystem(FileSystemVisitor &Visitor)
    {
        if( FileStream )
        {
            VisitBlock(2, Visitor);
        }
    }

    void VirtualFileSystem::IterateCanvas(CanvasVisitor &Visitor)
    {
        if( FileStream )
        {
            FileEntry Canvas;
            if( GetEntry("canvas", Canvas) == false )
            {
                return;
            }
            Canvas.Seek(4);
            Visitor.VisitCanvasBegin(
                Canvas.Read<uint32_t>(),
                Canvas.Read<uint32_t>()
            );

            FileEntry LayerTable;
            if( GetEntry("laytbl", LayerTable) == false )
            {
                return;
            }

            uint32_t LayerCount = LayerTable.Read<uint32_t>();

            LayerTableEntry CurLayer;

            std::stack<uint32_t> SetStack;

            for( size_t i = 0; i < LayerCount; i++ )
            {
                if( LayerTable.Read<LayerTableEntry>(CurLayer) == false )
                {
                    return;
                }
                char LayerPath[0xff];
                std::sprintf(LayerPath, "/layers/%08x", CurLayer.Identifier);

                FileEntry LayerFile;
                if( GetEntry(LayerPath, LayerFile) == false )
                {
                    return;
                }

                LayerHeader LayerHead;
                if( LayerFile.Read<LayerHeader>(LayerHead) == false )
                {
                    return;
                }

                // Read all tag entries
                uint32_t CurTag, CurTagSize;
                CurTag = CurTagSize = 0;

                char Name[256];
                int32_t ParentFolder = -1;
                uint8_t Open = 0;

                while( LayerFile.Read<uint32_t>(CurTag) && CurTag )
                {
                    LayerFile.Read<uint32_t>(CurTagSize);

                    switch( CurTag )
                    {
                    case 'name':
                    {
                        LayerFile.Read<char[256]>(Name);
                        break;
                    }
                    case 'pfid': // Parent folder ID
                    {
                        LayerFile.Read<int32_t>(ParentFolder);
                        break;
                    }
                    case 'fopn': // Folder is open
                    {
                        LayerFile.Read<uint8_t>(Open);
                        break;
                    }
                    default:
                    {
                        printf("%c%c%c%c | %u\n",
                            reinterpret_cast<char*>(&CurTag)[3],
                            reinterpret_cast<char*>(&CurTag)[2],
                            reinterpret_cast<char*>(&CurTag)[1],
                            reinterpret_cast<char*>(&CurTag)[0],
                            CurTagSize
                        );
                        LayerFile.Seek(LayerFile.Tell() + CurTagSize);
                        break;
                    }
                    }
                }

                if( LayerHead.Type == LayerHeader::Type::Layer )
                {
                    while( SetStack.size() && SetStack.top() != ParentFolder )
                    {
                        SetStack.pop();
                        Visitor.VisitFolderEnd();
                    }
                    Visitor.VisitLayer(Name);
                }
                else if( LayerHead.Type == LayerHeader::Type::Linework )
                {
                    Visitor.VisitLineart(Name);
                }
                else if( LayerHead.Type == LayerHeader::Type::Set )
                {
                    Visitor.VisitFolderBegin(Name, Open != 0);
                    SetStack.push(LayerHead.Identifier);
                }
            }

            Visitor.VisitCanvasEnd();
        }
    }

    void VirtualFileSystem::VisitBlock(size_t BlockNumber, FileSystemVisitor &Visitor)
    {
        FileSystemBlock CurBlock;
        GetBlock(BlockNumber, &CurBlock);
        FileEntry CurEntry(*this);
        for( size_t i = 0; CurBlock.FATEntries[i].Flags; i++ )
        {
            CurEntry.Data = CurBlock.FATEntries[i];
            switch( CurEntry.GetType() )
            {
            case FileEntry::EntryType::File:
            {
                Visitor.VisitFile(CurEntry);
                break;
            }
            case FileEntry::EntryType::Folder:
            {
                Visitor.VisitFolderBegin(CurEntry);
                VisitBlock(CurEntry.GetBlock(), Visitor);
                Visitor.VisitFolderEnd();
                break;
            }
            }
        }
    }

    bool VirtualFileSystem::GetBlock(size_t BlockNum, FileSystemBlock *Block)
    {
        if( BlockNum < BlockCount )
        {
            if( BlockNum & 0x1FF ) // Block is data
            {
                size_t NearestTable = BlockNum & ~(0x1FF);
                uint32_t Key = 0;
                if( CacheTableNum == NearestTable ) // Table Cache Hit
                {
                    Key = CacheTable->TableEntries[BlockNum - NearestTable].Checksum;
                }
                else // Cache Miss
                {
                    // Read and Decrypt Table
                    FileStream.seekg(NearestTable * FileSystemBlock::BlockSize);
                    FileStream.read(
                        reinterpret_cast<char*>(CacheTable->u8),
                        FileSystemBlock::BlockSize
                    );
                    CacheTable->DecryptTable(static_cast<uint32_t>(NearestTable));
                    Key = CacheTable->TableEntries[BlockNum - NearestTable].Checksum;
                }

                // Read and Decrypt Data
                FileStream.seekg(BlockNum * FileSystemBlock::BlockSize);
                FileStream.read(
                    reinterpret_cast<char*>(Block->u8),
                    FileSystemBlock::BlockSize
                );
                Block->DecryptData(Key);
                return true;
            }
            else // Block is a table
            {
                if( BlockNum == CacheTableNum ) // Cache hit
                {
                    memcpy(Block->u8, CacheTable->u8, FileSystemBlock::BlockSize);
                    return true;
                }
                // Read and Decrypt Table
                FileStream.seekg(BlockNum * FileSystemBlock::BlockSize);
                FileStream.read(
                    reinterpret_cast<char*>(CacheTable->u8),
                    FileSystemBlock::BlockSize
                );
                CacheTable->DecryptTable(static_cast<uint32_t>(BlockNum));
                CacheTableNum = BlockNum;
                memcpy(Block->u8, CacheTable->u8, FileSystemBlock::BlockSize);
                return true;
            }
        }
        return false;
    }

    // Block

    void VirtualBlock::DecryptTable(uint32_t BlockNumber)
    {
        BlockNumber &= (~0x1FF);
        for( size_t i = 0; i < (BlockSize / sizeof(uint32_t)); i++ )
        {
            uint32_t CurCipher = u32[i];
            uint32_t X = BlockNumber ^ CurCipher ^ (
                DecryptionKey[(BlockNumber >> 24) & 0xFF]
                + DecryptionKey[(BlockNumber >> 16) & 0xFF]
                + DecryptionKey[(BlockNumber >> 8) & 0xFF]
                + DecryptionKey[BlockNumber & 0xFF]);

            u32[i] = static_cast<uint32_t>((X << 16) | (X >> 16));

            BlockNumber = CurCipher;
        };
    }

    void VirtualBlock::DecryptData(uint32_t Key)
    {
        for( size_t i = 0; i < (BlockSize / sizeof(uint32_t)); i++ )
        {
            uint32_t CurCipher = u32[i];
            u32[i] =
                CurCipher
                - (Key ^ (
                    DecryptionKey[Key & 0xFF]
                    + DecryptionKey[(Key >> 8) & 0xFF]
                    + DecryptionKey[(Key >> 16) & 0xFF]
                    + DecryptionKey[(Key >> 24) & 0xFF]));
            Key = CurCipher;
        }
    }

    uint32_t VirtualBlock::Checksum(bool Table)
    {
        uint32_t Accumulate = 0;
        for( size_t i = (Table ? 1 : 0); i < (BlockSize / sizeof(uint32_t)); i++ )
        {
            Accumulate = (2 * Accumulate | (Accumulate >> 31)) ^ u32[i];
        }
        return Accumulate | 1;
    }

    const uint32_t VirtualBlock::DecryptionKey[256] =
    {
        0x9913D29E,0x83F58D3D,0xD0BE1526,0x86442EB7,0x7EC69BFB,0x89D75F64,0xFB51B239,0xFF097C56,
        0xA206EF1E,0x973D668D,0xC383770D,0x1CB4CCEB,0x36F7108B,0x40336BCD,0x84D123BD,0xAFEF5DF3,
        0x90326747,0xCBFFA8DD,0x25B94703,0xD7C5A4BA,0xE40A17A0,0xEADAE6F2,0x6B738250,0x76ECF24A,
        0x6F2746CC,0x9BF95E24,0x1ECA68C5,0xE71C5929,0x7817E56C,0x2F99C471,0x395A32B9,0x61438343,
        0x5E3E4F88,0x80A9332C,0x1879C69F,0x7A03D354,0x12E89720,0xF980448E,0x03643576,0x963C1D7B,
        0xBBED01D6,0xC512A6B1,0x51CB492B,0x44BADEC9,0xB2D54BC1,0x4E7C2893,0x1531C9A3,0x43A32CA5,
        0x55B25A87,0x70D9FA79,0xEF5B4AE3,0x8AE7F495,0x923A8505,0x1D92650C,0xC94A9A5C,0x27D4BB14,
        0x1372A9F7,0x0C19A7FE,0x64FA1A53,0xF1A2EB6D,0x9FEB910F,0x4CE10C4E,0x20825601,0x7DFC98C4,
        0xA046C808,0x8E90E7BE,0x601DE357,0xF360F37C,0x00CD6F77,0xCC6AB9D4,0x24CC4E78,0xAB1E0BFC,
        0x6A8BC585,0xFD70ABF0,0xD4A75261,0x1ABF5834,0x45DCFE17,0x5F67E136,0x948FD915,0x65AD9EF5,
        0x81AB20E9,0xD36EAF42,0x0F7F45C7,0x1BAE72D9,0xBE116AC6,0xDF58B4D5,0x3F0B960E,0xC2613F98,
        0xB065F8B0,0x6259F975,0xC49AEE84,0x29718963,0x0B6D991D,0x09CF7A37,0x692A6DF8,0x67B68B02,
        0x2E10DBC2,0x6C34E93C,0xA84B50A1,0xAC6FC0BB,0x5CA6184C,0x34E46183,0x42B379A9,0x79883AB6,
        0x08750921,0x35AF2B19,0xF7AA886A,0x49F281D3,0xA1768059,0x14568CFD,0x8B3625F6,0x3E1B2D9D,
        0xF60E14CE,0x1157270A,0xDB5C7EB3,0x738A0AFA,0x19C248E5,0x590CBD62,0x7B37C312,0xFC00B148,
        0xD808CF07,0xD6BD1C82,0xBD50F1D8,0x91DEA3B8,0xFA86B340,0xF5DF2A80,0x9A7BEA6E,0x1720B8F1,
        0xED94A56B,0xBF02BE28,0x0D419FA8,0x073B4DBC,0x829E3144,0x029F43E1,0x71E6D51F,0xA9381F09,
        0x583075E0,0xE398D789,0xF0E31106,0x75073EB5,0x5704863E,0x6EF1043B,0xBC407F33,0x8DBCFB25,
        0x886C8F22,0x5AF4DD7A,0x2CEACA35,0x8FC969DC,0x9DB8D6B4,0xC65EDC2F,0xE60F9316,0x0A84519A,
        0x3A294011,0xDCF3063F,0x41621623,0x228CB75B,0x28E9D166,0xAE631B7F,0x06D8C267,0xDA693C94,
        0x54A5E860,0x7C2170F4,0xF2E294CB,0x5B77A0F9,0xB91522A6,0xEC549500,0x10DD78A7,0x3823E458,
        0x77D3635A,0x018E3069,0xE039D055,0xD5C341BF,0x9C2400EA,0x85C0A1D1,0x66059C86,0x0416FF1A,
        0xE27E05C8,0xB19C4C2D,0xFE4DF58F,0xD2F0CE2A,0x32E013C0,0xEED637D7,0xE9FEC1E8,0xA4890DCA,
        0xF4180313,0x7291738C,0xE1B053A2,0x9801267E,0x2DA15BDB,0xADC4DA4F,0xCF95D474,0xC0265781,
        0x1F226CED,0xA7472952,0x3C5F0273,0xC152BA68,0xDD66F09B,0x93C7EDCF,0x4F147404,0x3193425D,
        0x26B5768A,0x0E683B2E,0x952FDF30,0x2A6BAE46,0xA3559270,0xB781D897,0xEB4ECB51,0xDE49394D,
        0x483F629C,0x2153845E,0xB40D64E2,0x47DB0ED0,0x302D8E4B,0x4BF8125F,0x2BD2B0AC,0x3DC836EC,
        0xC7871965,0xB64C5CDE,0x9EA8BC27,0xD1853490,0x3B42EC6F,0x63A4FD91,0xAA289D18,0x4D2B1E49,
        0xB8A060AD,0xB5F6C799,0x6D1F7D1C,0xBA8DAAE6,0xE51A0FC3,0xD94890E7,0x167DF6D2,0x879BCD41,
        0x5096AC1B,0x05ACB5DA,0x375D24EE,0x7F2EB6AA,0xA535F738,0xCAD0AD10,0xF8456E3A,0x23FD5492,
        0xB3745532,0x53C1A272,0x469DFCDF,0xE897BF7D,0xA6BBE2AE,0x68CE38AF,0x5D783D0B,0x524F21E4,
        0x4A257B31,0xCE7A07B2,0x562CE045,0x33B708A4,0x8CEE8AEF,0xC8FB71FF,0x74E52FAB,0xCDB18796,
    };
}
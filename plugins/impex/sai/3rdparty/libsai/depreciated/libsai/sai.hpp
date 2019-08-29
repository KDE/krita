#pragma once

#include <stdint.h>
#include <memory>
#include <fstream>

namespace sai
{
    /// Prototypes
    class VirtualFileSystem;
    union VirtualBlock;

    /// File Entry
    class VirtualFileEntry
    {
        friend class VirtualFileSystem;
        friend union VirtualBlock;
    public:
        VirtualFileEntry();
        VirtualFileEntry(VirtualFileSystem &FileSystem);
        ~VirtualFileEntry();
        uint32_t GetFlags() const;
        const char* GetName() const;

        enum class EntryType : uint8_t
        {
            Folder = 0x10,
            File = 0x80
        };

        EntryType GetType() const;
        size_t GetBlock() const;
        size_t GetSize() const;
        time_t GetTimeStamp() const;

        size_t Tell() const;
        void Seek(size_t Offset);

        bool Read(void *Destination, size_t Size);

        template< typename T >
        inline bool Read(T &Destination)
        {
            return Read(&Destination, sizeof(T));
        }

        template< typename T >
        inline T Read()
        {
            T temp;
            Read(&temp, sizeof(T));
            return temp;
        }

    private:
        VirtualFileSystem *FileSystem;
        size_t Position;

#pragma pack(push, 1)
        struct FATEntry
        {
            uint32_t Flags;
            char Name[32];
            uint8_t Pad1;
            uint8_t Pad2;
            EntryType Type;
            uint8_t Pad4;
            uint32_t Block;
            uint32_t Size;
            uint64_t TimeStamp; // Windows FILETIME
            uint64_t UnknownB;
        } Data;
#pragma pack(pop)
    };

    typedef VirtualFileEntry FileEntry;

    /// File System Visitor
    class VFSVisitor
    {
    public:
        virtual ~VFSVisitor() {};

        // Visit a Folder
        virtual void VisitFolderBegin(FileEntry &Entry) = 0;
        virtual void VisitFolderEnd() = 0;

        // Visit a File
        virtual void VisitFile(FileEntry &Entry) = 0;
    };

    typedef VFSVisitor FileSystemVisitor;

    /// File system Block (4096 bytes)
#pragma pack(push, 1)
    union VirtualBlock
    {
        static const size_t BlockSize = 0x1000;
        // Decryption key
        static const uint32_t DecryptionKey[256];

        // Data
        uint8_t u8[4096];
        uint32_t u32[1024];

        // Block Table entries
        struct TableEntry
        {
            uint32_t Checksum;
            uint32_t Flags;
        } TableEntries[512];

        // File allocation Entries
        FileEntry::FATEntry FATEntries[64];

        void DecryptTable(uint32_t Index);
        void DecryptData(uint32_t Key);

        uint32_t Checksum(bool Table = false);
    };
#pragma pack(pop)

    typedef VirtualBlock FileSystemBlock;

    /// Canvas Visitor
    class CanvasVisitor
    {
    public:
        virtual ~CanvasVisitor() {};

        // Ran before and after a canvas is being iterated
        virtual void VisitCanvasBegin(size_t Width, size_t Height) = 0;
        virtual void VisitCanvasEnd() = 0;

        // Visit a Layer folder/set
        virtual void VisitFolderBegin(const char* FolderName, bool Open) = 0;
        virtual void VisitFolderEnd() = 0;

        // Visit Layer
        virtual void VisitLayer(const char* LayerName) = 0;

        // Visit Lineart
        virtual void VisitLineart(const char* LayerName) = 0;
    };

    /// File System
    class VirtualFileSystem
    {
    public:
        VirtualFileSystem();

        /// Noncopyable
        VirtualFileSystem(const VirtualFileSystem&) = delete;
        VirtualFileSystem& operator=(const VirtualFileSystem&) = delete;

        ~VirtualFileSystem();

        bool Mount(const char *FileName);

        size_t GetBlockCount() const;

        size_t GetSize() const;

        bool GetEntry(const char *Path, FileEntry &Entry);

        bool Read(size_t Offset, size_t Size, void *Destination);

        template< typename T >
        inline bool Read(size_t Offset, T &Data)
        {
            return Read(Offset, sizeof(T), &Data);
        }

        /// Iterators
        void IterateFileSystem(FileSystemVisitor &Visitor);
        void IterateCanvas(CanvasVisitor &Visitor);

    private:
        void VisitBlock(size_t BlockNumber, FileSystemVisitor &Visitor);
        bool GetBlock(size_t BlockNum, FileSystemBlock *Block);

        /// Current VFS context
        size_t BlockCount;
        std::ifstream FileStream;

        // Block Caching
        intmax_t CacheTableNum = -1;
        std::unique_ptr<FileSystemBlock> CacheTable;
        std::unique_ptr<FileSystemBlock> CacheBuffer;
    };

    typedef VirtualFileSystem FileSystem;
}
#include <sai.hpp>

#include <fstream>
#include <algorithm>
#include <cstring>
#include <codecvt>
#include <locale>

#include <immintrin.h>

namespace sai
{
/// Internal Structures

#pragma pack(push, 1)

struct ThumbnailHeader
{
	std::uint32_t Width;
	std::uint32_t Height;
	std::uint32_t Magic; // BM32
};

enum class LayerClass
{
	RootLayer = 0x00,
	// Parent Canvas layer object
	Layer = 0x03,
	Unknown4 = 0x4,
	Linework = 0x05,
	Mask = 0x06,
	Unknown7 = 0x07,
	Set = 0x08
};

struct LayerReference
{
	std::uint32_t Identifier;
	std::uint16_t LayerClass;
	// These all get added and sent as a windows message 0x80CA for some reason
	std::uint16_t Unknown;
};

struct LayerBounds
{
	std::int32_t X; // (X / 32) * 32
	std::int32_t Y; // (Y / 32) * 32
	std::uint32_t Width; // Width - 31
	std::uint32_t Height; // Height - 31
};

struct LayerHeader
{
	std::uint32_t LayerClass;
	std::uint32_t Identifier;
	LayerBounds Bounds;
	std::uint32_t Unknown;
	std::uint8_t Opacity;
	std::uint8_t Visible;
	std::uint8_t PreserveOpacity;
	std::uint8_t Clipping;
	std::uint8_t Unknown4;
	std::uint32_t Blending;
};

#pragma pack(pop)

/// VirtualPage
#if defined(__AVX2__)
inline __m256i KeySum8(
	__m256i Vector8, const std::uint32_t Key[256]
)
{
	__m256i Sum = _mm256_i32gather_epi32(
		(const std::int32_t*)Key,
		_mm256_and_si256(Vector8, _mm256_set1_epi32(0xFF)),
		sizeof(std::uint32_t)
	);

	Sum = _mm256_add_epi32(
		Sum,
		_mm256_i32gather_epi32(
			(const std::int32_t*)Key,
			_mm256_and_si256(
				_mm256_srli_epi32(Vector8, 8), _mm256_set1_epi32(0xFF)
			),
			sizeof(std::uint32_t)
		)
	);
	Sum = _mm256_add_epi32(
		Sum,
		_mm256_i32gather_epi32(
			(const std::int32_t*)Key,
			_mm256_and_si256(
				_mm256_srli_epi32(Vector8, 16), _mm256_set1_epi32(0xFF)
			),
			sizeof(std::uint32_t)
		)
	);
	Sum = _mm256_add_epi32(
		Sum,
		_mm256_i32gather_epi32(
			(const std::int32_t*)Key, _mm256_srli_epi32(Vector8, 24),
			sizeof(std::uint32_t)
		)
	);
	return Sum;
}
#endif

void VirtualPage::DecryptTable(std::uint32_t PageIndex)
{
	std::uint32_t PrevData = PageIndex & (~0x1FF);
	#if defined(__AVX2__)
	__m256i PrevData8 = _mm256_set1_epi32(PrevData);
	for( std::size_t i = 0; i < (PageSize / sizeof(std::uint32_t)); i += 8 )
	{
		const __m256i CurData8 = _mm256_loadu_si256((__m256i*)(u32 + i));
		// There is no true _mm_alignr_epi8 for AVX2
		// An extra _mm256_permute2x128_si256 is needed
		PrevData8 = _mm256_alignr_epi8(
			CurData8,
			_mm256_permute2x128_si256(PrevData8, CurData8, _MM_SHUFFLE(0,2,0,1)),
			sizeof(std::uint32_t) * 3
		);
		__m256i CurPlain8 = _mm256_xor_si256(
			_mm256_xor_si256(CurData8, PrevData8),
			KeySum8(PrevData8, Keys::User)
		);
		CurPlain8 = _mm256_shuffle_epi8(
			CurPlain8,
			_mm256_set_epi8(
				13, 12, 15, 14, 9,  8, 11, 10, 5,  4,  7,  6, 1,  0,  3,  2,
				13, 12, 15, 14, 9,  8, 11, 10, 5,  4,  7,  6, 1,  0,  3,  2
			)
		);
		_mm256_storeu_si256((__m256i*)(u32 + i), CurPlain8);
		PrevData8 = CurData8;
	};
	#else
	for( std::size_t i = 0; i < (PageSize / sizeof(std::uint32_t)); i++ )
	{
		const std::uint32_t CurData = u32[i];
		std::uint32_t X = PrevData ^ CurData;
		X ^= (
			  Keys::User[(PrevData >> 24) & 0xFF]
			+ Keys::User[(PrevData >> 16) & 0xFF]
			+ Keys::User[(PrevData >>  8) & 0xFF]
			+ Keys::User[(PrevData >>  0) & 0xFF]
		);
		u32[i] = static_cast<std::uint32_t>((X << 16) | (X >> 16));
		PrevData = CurData;
	};
	#endif
}

void VirtualPage::DecryptData(std::uint32_t PageChecksum)
{
	std::uint32_t PrevData = PageChecksum;
	#if defined(__AVX2__)
	__m256i PrevData8 = _mm256_set1_epi32(PrevData);
	for( std::size_t i = 0; i < (PageSize / sizeof(std::uint32_t)); i += 8 )
	{
		const __m256i CurData8 = _mm256_loadu_si256((__m256i*)(u32 + i));
		// There is no true _mm_alignr_epi8 for AVX2
		// An extra _mm256_permute2x128_si256 is needed
		PrevData8 = _mm256_alignr_epi8(
			CurData8,
			_mm256_permute2x128_si256(PrevData8, CurData8, _MM_SHUFFLE(0,2,0,1)),
			sizeof(std::uint32_t) * 3
		);
		__m256i CurPlain8 = _mm256_sub_epi32(
			CurData8,
			_mm256_xor_si256(PrevData8, KeySum8(PrevData8, Keys::User))
		);
		_mm256_storeu_si256((__m256i*)(u32 + i), CurPlain8);
		PrevData8 = CurData8;
	};
	#else
	for( std::size_t i = 0; i < (PageSize / sizeof(std::uint32_t)); i++ )
	{
		const std::uint32_t CurData = u32[i];
		u32[i] =
			CurData
			- (PrevData ^ (
				  Keys::User[(PrevData >> 24) & 0xFF]
				+ Keys::User[(PrevData >> 16) & 0xFF]
				+ Keys::User[(PrevData >>  8) & 0xFF]
				+ Keys::User[(PrevData >>  0) & 0xFF]
				)
			);
		PrevData = CurData;
	}
	#endif
}

std::uint32_t VirtualPage::Checksum()
{
	std::uint32_t Sum = 0;
	for( std::size_t i = 0; i < (PageSize / sizeof(std::uint32_t)); i++ )
	{
		Sum = (2 * Sum | (Sum >> 31)) ^ u32[i];
	}
	return Sum | 1;
}

/// ifstreambuf
ifstreambuf::ifstreambuf(const std::uint32_t* Key)
	: Key(Key),
	CurrentPage(-1),
	PageCache(nullptr),
	PageCacheIndex(-1),
	TableCache(nullptr),
	TableCacheIndex(-1),
	PageCount(0)
{
	setg(
		nullptr,
		nullptr,
		nullptr
	);

	setp(
		nullptr,
		nullptr
	);

	PageCache = std::make_unique<VirtualPage>();
	TableCache = std::make_unique<VirtualPage>();
}

ifstreambuf* ifstreambuf::open(const char* Name)
{
	if( is_open() == true )
	{
		return nullptr;
	}

	FileIn.open(
		Name,
		std::ios_base::binary | std::ios_base::ate
	);

	if( FileIn.is_open() == false )
	{
		close();
		return nullptr;
	}

	const std::ifstream::pos_type FileSize = FileIn.tellg();

	if( FileSize % VirtualPage::PageSize != 0 )
	{
		// File size is not pagealigned
		close();
		return nullptr;
	}

	PageCount = static_cast<std::uint32_t>(FileSize) / VirtualPage::PageSize;

	seekpos(
		0
	);

	return this;
}

ifstreambuf* ifstreambuf::open(const wchar_t* Name)
{
	if( is_open() == true )
	{
		return nullptr;
	}

#if defined(_WIN32)
	FileIn.open(
		Name,
		std::ios_base::binary | std::ios_base::ate
	);
#else
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> Converter;
	std::string Name8 = Converter.to_bytes(std::wstring(Name));
	FileIn.open(
		Name8,
		std::ios_base::binary | std::ios_base::ate
	);
#endif

	if( FileIn.is_open() == false )
	{
		close();
		return nullptr;
	}

	const std::ifstream::pos_type FileSize = FileIn.tellg();

	if( FileSize % VirtualPage::PageSize != 0 )
	{
		// File size is not pagealigned
		close();
		return nullptr;
	}

	PageCount = static_cast<std::uint32_t>(FileSize) / VirtualPage::PageSize;

	seekpos(
		0
	);

	return this;
}

ifstreambuf* ifstreambuf::close()
{
	if( FileIn.is_open() )
	{
		FileIn.close();
		return this;
	}
	return nullptr;
}

bool ifstreambuf::is_open() const
{
	return FileIn.is_open();
}

std::streambuf::int_type ifstreambuf::underflow()
{
	if( FileIn.eof() )
	{
		return traits_type::eof();
	}

	if( gptr() == egptr() )
	{
		// buffer depleated, get next block
		if( seekpos(
				(CurrentPage + 1) * VirtualPage::PageSize
			) == std::streampos(std::streamoff(-1))
		)
		{
			// Seek position error
			return traits_type::eof();
		}
	}

	return traits_type::to_int_type(*gptr());
}

std::streambuf::pos_type ifstreambuf::seekoff(
	std::streambuf::off_type Offset,
	std::ios_base::seekdir Direction,
    std::ios_base::openmode /*Mode*/
)
{
	std::streambuf::pos_type Position;

	if( Direction & std::ios_base::beg )
	{
		Position = (CurrentPage * VirtualPage::PageSize); // Current Page
		Position += (gptr() - egptr()); // Offset within page
		Position += Offset;
	}
	if( Direction & std::ios_base::cur )
	{
		Position = Offset;
	}
	if( Direction & std::ios_base::end )
	{
		Position = (PageCount * VirtualPage::PageSize) + Offset;
	}

	return seekpos(
		Position
	);
}

std::streambuf::pos_type ifstreambuf::seekpos(
	std::streambuf::pos_type Position,
	std::ios_base::openmode Mode
)
{
	if( Mode & std::ios_base::in )
	{
		CurrentPage = static_cast<std::uint32_t>(Position) / VirtualPage::PageSize;

		if( CurrentPage < PageCount )
		{
			if( FetchPage(CurrentPage, &Buffer) )
			{
				setg(
					reinterpret_cast<char*>(Buffer.u8),
					reinterpret_cast<char*>(Buffer.u8) + (Position % VirtualPage::PageSize),
					reinterpret_cast<char*>(Buffer.u8) + VirtualPage::PageSize
				);
				return true;
			}
		}
	}
	setg(
		nullptr,
		nullptr,
	nullptr
	);
	return std::streampos(std::streamoff(-1));
}

bool ifstreambuf::FetchPage(std::uint32_t PageIndex, VirtualPage* Dest)
{
	if( FileIn.fail() )
	{
		return false;
	}

	if( PageIndex % VirtualPage::TableSpan == 0 ) // Table Block
	{
		if( PageIndex == TableCacheIndex )
		{
			// Cache Hit
			if( Dest != nullptr )
			{
				std::memcpy(
					Dest,
					TableCache.get(),
					VirtualPage::PageSize
				);
			}
			return true;
		}

		FileIn.seekg(
			PageIndex * VirtualPage::PageSize,
			std::ios_base::beg
		);
		FileIn.read(
			reinterpret_cast<char*>(TableCache.get()),
			VirtualPage::PageSize
		);
		if( FileIn.fail() )
		{
			return false;
		}
		TableCache.get()->DecryptTable(PageIndex);
		TableCacheIndex = PageIndex;
		if( Dest != nullptr )
		{
			std::memcpy(
				Dest,
				TableCache.get(),
				VirtualPage::PageSize
			);
		}
	}
	else // Data Block
	{
		if( PageIndex == PageCacheIndex )
		{
			// Cache Hit
			if( Dest != nullptr )
			{
				std::memcpy(
					Dest,
					PageCache.get(),
					VirtualPage::PageSize
				);
			}
			return true;
		}
		// Prefetch nearest table
		// Ensure it is in the cache
		const std::uint32_t NearestTable
			= (PageIndex / VirtualPage::TableSpan) * VirtualPage::TableSpan;

		if( FetchPage(NearestTable, nullptr) == false )
		{
			// Failed to fetch table
			return false;
		}
		FileIn.seekg(
			PageIndex * VirtualPage::PageSize,
			std::ios_base::beg
		);
		FileIn.read(
			reinterpret_cast<char*>(PageCache.get()),
			VirtualPage::PageSize
		);
		if( FileIn.fail() )
		{
			return false;
		}
		PageCache.get()->DecryptData(
			TableCache.get()->PageEntries[PageIndex % VirtualPage::TableSpan].Checksum
		);

		if(
			PageCache.get()->Checksum()
			!=
			TableCache.get()->PageEntries[PageIndex % VirtualPage::TableSpan].Checksum
		)
		{
			// Checksum mismatch, file corrupt
			return false;
		}

		PageCacheIndex = PageIndex;
		if( Dest != nullptr )
		{
			std::memcpy(
				Dest,
				PageCache.get(),
				VirtualPage::PageSize
			);
		}
	}
	return true;
}

/// ifstream
ifstream::ifstream(const std::string& Path)
	: std::istream(new ifstreambuf())
{
	reinterpret_cast<ifstreambuf*>(rdbuf())->open(
		Path.c_str()
	);
}

ifstream::ifstream(const char* Path)
	: std::istream(new ifstreambuf())
{
	reinterpret_cast<ifstreambuf*>(rdbuf())->open(
		Path
	);
}

ifstream::ifstream(const std::wstring& Path)
	: std::istream(new ifstreambuf())
{
	reinterpret_cast<ifstreambuf*>(rdbuf())->open(
		Path.c_str()
	);
}

ifstream::ifstream(const wchar_t* Path)
	: std::istream(new ifstreambuf())
{
	reinterpret_cast<ifstreambuf*>(rdbuf())->open(
		Path
	);
}

void ifstream::open(const char* FilePath) const
{
	reinterpret_cast<ifstreambuf*>(rdbuf())->close();
	reinterpret_cast<ifstreambuf*>(rdbuf())->open(
		FilePath
	);
}

void ifstream::open(const std::string& FilePath) const
{
	open(FilePath.c_str());
}

void ifstream::open(const wchar_t* FilePath) const
{
	reinterpret_cast<ifstreambuf*>(rdbuf())->close();
	reinterpret_cast<ifstreambuf*>(rdbuf())->open(
		FilePath
	);
}

void ifstream::open(const std::wstring& FilePath) const
{
	open(FilePath.c_str());
}


bool ifstream::is_open() const
{
	return reinterpret_cast<ifstreambuf*>(rdbuf())->is_open();
}

ifstream::~ifstream()
{
	if( rdbuf() )
	{
		delete rdbuf();
	}
}

VirtualFileVisitor::~VirtualFileVisitor()
{
}

bool VirtualFileVisitor::VisitFolderBegin(VirtualFileEntry& /*Entry*/)
{
	return true;
}

bool VirtualFileVisitor::VisitFolderEnd(VirtualFileEntry& /*Entry*/)
{
	return true;
}

bool VirtualFileVisitor::VisitFile(VirtualFileEntry& /*Entry*/)
{
	return true;
}

/// Virtual File System

VirtualFileSystem::VirtualFileSystem(const char* FileName)
	: SaiStream(std::make_shared<ifstream>(FileName))
{
}

VirtualFileSystem::VirtualFileSystem(const wchar_t* FileName)
	: SaiStream(std::make_shared<ifstream>(FileName))
{
}

VirtualFileSystem::~VirtualFileSystem()
{
}

bool VirtualFileSystem::IsOpen() const
{
	return SaiStream->is_open();
}

bool VirtualFileSystem::Exists(const char* Path)
{
	return static_cast<bool>(GetEntry(Path));
}

std::unique_ptr<VirtualFileEntry> VirtualFileSystem::GetEntry(const char* Path)
{
	VirtualPage CurPage;
	Read(
		2 * VirtualPage::PageSize,
		CurPage
	);

	std::string CurPath(Path);
    const char* PathDelim = "./";

	const char* CurToken = std::strtok(&CurPath[0], PathDelim);

	std::size_t CurEntry = 0;

	while( CurEntry < 64 && CurPage.FATEntries[CurEntry].Flags && CurToken )
	{
		if( std::strcmp(CurToken, CurPage.FATEntries[CurEntry].Name) == 0 )
		{
			// Match
			if( (CurToken = std::strtok(nullptr, PathDelim)) == nullptr )
			{
				// No more tokens, done
				std::unique_ptr<VirtualFileEntry> Entry(new VirtualFileEntry());

				Entry->FATData = CurPage.FATEntries[CurEntry];
				Entry->FileSystem = SaiStream;
				return Entry;
			}
			// Try to go further
			if( CurPage.FATEntries[CurEntry].Type != FATEntry::EntryType::Folder )
			{
				// Part of the path was not a folder, cant go further
				return nullptr;
			}
			Read(
				CurPage.FATEntries[CurEntry].PageIndex * VirtualPage::PageSize,
				CurPage
			);
			CurEntry = 0;
			continue;
		}
		CurEntry++;
	}

	return nullptr;
}

std::size_t VirtualFileSystem::Read(
	std::size_t Offset,
	void* Destination,
	std::size_t Size) const
{
	SaiStream->seekg(Offset);
	SaiStream->read(
		reinterpret_cast<char*>(Destination),
		Size
	);
	return Size;
}

void VirtualFileSystem::IterateFileSystem(VirtualFileVisitor& Visitor)
{
	IterateFATBlock(
		2,
		Visitor
	);
}

void VirtualFileSystem::IterateFATBlock(
	std::size_t PageIndex,
	VirtualFileVisitor& Visitor
)
{
	VirtualPage CurPage;
	Read(
		PageIndex * VirtualPage::PageSize,
		CurPage
	);

	for( std::size_t i = 0; CurPage.FATEntries[i].Flags; i++ )
	{
		VirtualFileEntry CurEntry;
		CurEntry.FATData = CurPage.FATEntries[i];
		CurEntry.FileSystem = SaiStream;
		switch( CurEntry.GetType() )
		{
		case FATEntry::EntryType::File:
		{
			Visitor.VisitFile(CurEntry);
			break;
		}
		case FATEntry::EntryType::Folder:
		{
			Visitor.VisitFolderBegin(CurEntry);
			IterateFATBlock(
				CurEntry.GetPageIndex(),
				Visitor
			);
			Visitor.VisitFolderEnd(CurEntry);
			break;
		}
		}
	}
}

/// VirtualFileEntry
VirtualFileEntry::VirtualFileEntry()
	: ReadPoint(0),
	FATData()
{
}

VirtualFileEntry::~VirtualFileEntry()
{
}

const char* VirtualFileEntry::GetName() const
{
	return FATData.Name;
}

FATEntry::EntryType VirtualFileEntry::GetType() const
{
	return FATData.Type;
}

std::time_t VirtualFileEntry::GetTimeStamp() const
{
	return FATData.TimeStamp / 10000000ULL - 11644473600ULL;
}

std::size_t VirtualFileEntry::GetSize() const
{
	return static_cast<std::size_t>(FATData.Size);
}

std::size_t VirtualFileEntry::GetPageIndex() const
{
	return static_cast<std::size_t>(FATData.PageIndex);
}

std::size_t VirtualFileEntry::Tell() const
{
	return ReadPoint;
}

void VirtualFileEntry::Seek(std::size_t Offset)
{
	ReadPoint = Offset;
}

std::size_t VirtualFileEntry::Read(void* Destination, std::size_t Size)
{
	if( std::shared_ptr<ifstream> SaiStream = FileSystem.lock() )
	{
		// Because this is a high-level file-level read:
		// all table blocks must be abstracted away and "skipped"
		// or else we will be reading table-block data as file-data
		// this is a mess right now until I become more aware of some more sound
		// logic to do this.
		// What this is basically trying to do is skip reading every table block(blocks with index 0,512,1024,etc)
		// so this would need to SKIP all byte-ranges:
		// [0,4096],[2097152,2101248],[4194304,4198400],[TableIndex * 4096,TableIndex * 4096 + 4096]
		// and by skipping this then file-reads will appear to be perfectly continuous.
		// If you're reading this and have to work with this I'm so sorry.
		//												- Wunkolo, 10/19/17
		std::uint8_t* CurDest = reinterpret_cast<std::uint8_t*>(Destination);
		std::size_t NextTableIndex = ((ReadPoint + (FATData.PageIndex * VirtualPage::PageSize)) / VirtualPage::PageSize & ~(
			0x1FF)) + VirtualPage::TableSpan;
		while( Size )
		{
			// Requested offset that we want to read from
			const std::size_t CurOffset = ReadPoint + (FATData.PageIndex * VirtualPage::PageSize);
			const std::size_t CurrentIndex = CurOffset / VirtualPage::PageSize;
			// If we find ourselves within a table block, skip.
			if( NextTableIndex == CurrentIndex )
			{
				// Align to immediately at end of current block
				ReadPoint += VirtualPage::PageSize - (ReadPoint % VirtualPage::PageSize);
				NextTableIndex += VirtualPage::TableSpan;
				continue;
			}
			const std::size_t NextTableOffset = NextTableIndex * VirtualPage::PageSize;
			const std::size_t CurStride = std::min<std::size_t>(
				Size,
				NextTableOffset - CurOffset
			);
			SaiStream->seekg(CurOffset);
			SaiStream->read(
				reinterpret_cast<char*>(CurDest),
				CurStride
			);
			ReadPoint += CurStride;
			CurDest += CurStride;
			Size -= CurStride;
		}
		return Size;
	}
	return 0;
}

/// SaiDocument
Document::Document(const char* FileName)
	: VirtualFileSystem(FileName)
{
}

Document::Document(const wchar_t* FileName)
	: VirtualFileSystem(FileName)
{
}

Document::~Document()
{
}

std::tuple<std::uint32_t, std::uint32_t> Document::GetCanvasSize()
{
	if( std::unique_ptr<VirtualFileEntry> Canvas = GetEntry("canvas") )
	{
		std::uint32_t Alignment; // Always seems to be 0x10, bpc? Alignment?
		std::uint32_t Width, Height;

		Canvas->Read(Alignment);
		Canvas->Read(Width);
		Canvas->Read(Height);
		return std::make_tuple(Width, Height);
	}
	return std::make_tuple(0, 0);
}

std::tuple<
	std::unique_ptr<std::uint8_t[]>,
	std::uint32_t,
	std::uint32_t
> Document::GetThumbnail()
{
	if( std::unique_ptr<VirtualFileEntry> Thumbnail = GetEntry("thumbnail") )
	{
		ThumbnailHeader Header;
		Thumbnail->Read(Header.Width);
		Thumbnail->Read(Header.Height);
		Thumbnail->Read(Header.Magic);

        if( Header.Magic != *(uint*)"23MB" )
		{
			return std::make_tuple(nullptr, 0, 0);
		}

		const std::size_t PixelCount = Header.Height * Header.Width;
		std::unique_ptr<std::uint8_t[]> Pixels
			= std::make_unique<std::uint8_t[]>(PixelCount * sizeof(std::uint32_t));

		Thumbnail->Read(
			Pixels.get(),
			PixelCount * sizeof(std::uint32_t)
		);

		//// BGRA to RGBA
		//std::size_t i = 0;

		//// Simd speedup, four pixels at a time
		//while( i < ((PixelCount * sizeof(std::uint32_t)) & ~0xF) )
		//{
		//	const __m128i Swizzle =
		//		_mm_set_epi8(
		//			15, 12, 13, 14,
		//			11, 8, 9, 10,
		//			7, 4, 5, 6,
		//			3, 0, 1, 2
		//		);

		//	__m128i QuadPixel = _mm_loadu_si128(
		//		reinterpret_cast<__m128i*>(&Pixels[i])
		//	);

		//	QuadPixel = _mm_shuffle_epi8(QuadPixel, Swizzle);

		//	_mm_store_si128(
		//		reinterpret_cast<__m128i*>(&Pixels[i]),
		//		QuadPixel
		//	);

		//	i += (sizeof(std::uint32_t) * 4);
		//}

		//for( ; i < PixelCount * sizeof(std::uint32_t); i += sizeof(std::uint32_t) )
		//{
		//	std::swap<std::uint8_t>(Pixels[i], Pixels[i + 2]);
		//}

		return std::make_tuple(std::move(Pixels), Header.Width, Header.Height);
	}
	return std::make_tuple(nullptr, 0, 0);
}

/// Keys
namespace Keys
{
const std::uint32_t User[256] =
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
	0x4A257B31,0xCE7A07B2,0x562CE045,0x33B708A4,0x8CEE8AEF,0xC8FB71FF,0x74E52FAB,0xCDB18796
};

const std::uint32_t NotRemoveMe[256] =
{
	0xA0C62B54,0x0374CB94,0xB3A53F76,0x5B772C6B,0xF2B92931,0x80F923A9,0x7A22EF7A,0x216C7582,
	0xEDFF8B71,0x8B0C6642,0xAF81AD2F,0x8E095A62,0x02926C0C,0xDD2F56B9,0xA3614155,0xF9AED6E4,
	0x079C3E5E,0xE6D9E1FD,0x256F165C,0x77280767,0x5D2037A1,0x3019B3CE,0xFC13CC15,0xF457C85F,
	0x728DF4E9,0x4405AA18,0x2AE0B950,0xE847316F,0xD69FA172,0x62F658E2,0xB0F21F89,0x8AFB852E,
	0x1A3E924A,0xDBAD0B48,0x88ECBD5A,0xC53FC908,0x81251757,0x57D53685,0x73F463A3,0x048F4B58,
	0xC36A46AC,0x9A8B6FBD,0x35DC9DC1,0xF76EABF5,0x9280D935,0xBFCC93FB,0x4B2BCA7D,0x60861DFC,
	0x7C548877,0x2EA46821,0x7136998F,0x5AD45EDF,0x019BA6EF,0x6FC598C7,0x1DF383EC,0x39BAC06D,
	0x5C3A5B1F,0x7827FB39,0x27FCA953,0x8601E843,0x6C429623,0xBA5DC127,0xCE659075,0x48291378,
	0x5EDA6B5B,0xE355AC99,0xCF8C704D,0x965E6A29,0xF5035103,0x20582702,0x1B7909DB,0xCA974452,
	0x7DB20E30,0x2807326C,0x2DF56D0E,0x084E9C41,0xA42DE39C,0x9170A5C3,0x9DB4F95D,0x53CA2068,
	0x3488FC6E,0xD1BB7AE8,0xC61F81C5,0x310857E5,0xEF1694EE,0xF63067B1,0x3E621B8B,0x22523BFF,
	0x0D37A4BA,0xCB83BECA,0x9BE78691,0xB7D84E2C,0x45A676DD,0x1F31F636,0x7FAB97C6,0x3CA15F33,
	0xFA6DB6FE,0x67DD72DC,0x6B8948FA,0x9849FF4B,0xBE452E79,0x38AF6E7F,0x8FE211A7,0x941728B4,
	0x63217749,0x70EF1280,0x13A9F201,0xACDB14A2,0x1184E73A,0x337E87B5,0xB6008EB7,0xC868C43C,
	0x85F7DC83,0xD35AD519,0xF87310ED,0xA7C0D29B,0x361D2DCF,0xC1D27C3F,0x9C78DFE0,0x2C4FD8C4,
	0x05357D9D,0x2B398964,0x182AC610,0xFD4A3873,0xE71E6416,0x842C4A05,0x5946F70F,0xB95FA366,
	0x1C0B71CB,0x50CEFA06,0xAB9DC211,0x659ABCAE,0xD2E17FE7,0x581A0365,0xA61BE0B0,0xD460B084,
	0xE21C5CF9,0x87B1D460,0x4DF8CF04,0x4C1573EA,0xCD967432,0xD58EBA12,0x5F2E9A3B,0x6A9955EB,
	0x55A391AF,0xEBC1EED5,0xB59E8C7C,0x1E825946,0xAA18A04E,0x6891EDF3,0x663C542D,0xC459D37E,
	0xC06453BC,0x460D223E,0x1690F8DE,0xC97580F7,0xA1F08D4F,0x56DE4381,0xEE06B5E3,0xC2FA05D1,
	0x3794B488,0xEACD428E,0x7B2362C2,0xE97FDE9F,0xBB4C60D2,0xE4B3E2AB,0x74C93909,0x76AA2FDA,
	0x9F049B7B,0x93BCDA8A,0x51BEC790,0x0FD6E4CC,0x8972E6AD,0xBCA70F40,0x405C2469,0x10673486,
	0xBD104C97,0x49381E0D,0x063B456A,0x23D02634,0x43ACEC9E,0xE50E49F8,0x197DBF1B,0x8DF1BB9A,
	0xB46B1CA6,0xD7E895A5,0xCC51A217,0xE1C2F196,0xDEB533C9,0x24FDC58D,0x32850822,0x12DF4DA8,
	0x90BD3500,0x97C7F320,0xDA3450F4,0x2F534059,0xDC7B3D63,0x95B6CD98,0x09BF19D6,0xA5D15DBF,
	0x42E47851,0xF07A021E,0x9ECB2A3D,0xE0C39F38,0x99714F95,0x3A5BEA4C,0xB2C4DD25,0xB13D47C0,
	0xAD418A0B,0x6DEAB81C,0x83EE25F2,0x3B26AE47,0xA8B018D3,0xFF76E5F1,0xA2ED0461,0x26119ED8,
	0x61EB0A74,0x15A2B187,0x4A93CE2A,0x7943A707,0x29E5B744,0x4E14F02B,0x0A698424,0xD9A03AE6,
	0xEC87D7C8,0xA94021B8,0x3D95D1CD,0x6E2415BE,0x52E3F592,0x64A83CD9,0x8263C31D,0x41B87EB6,
	0x8C50FD1A,0x47C80CD7,0xD844008C,0xB812E9AA,0x0B983013,0xFB7C520A,0x4F66FEBB,0x17E982D0,
	0x00FE6914,0xFE0FD028,0x0C328F93,0x75021AF6,0x3FE6AFB2,0x7E330DE1,0xDF8ADB45,0x14D37B37,
	0xD04D06A4,0x694B0156,0x0ECF6170,0xC756EBF0,0xF1B76526,0xF348A8B3,0xAE0A79A0,0x54D7B2D4
};

const std::uint32_t LocalState[256] =
{
	0x021CF107,0xE9253648,0x8AFBA619,0x8CF31842,0xBF40F860,0xA672F03E,0xFA2756AC,0x927B2E7E,
	0x1E37D3C4,0x7C3A0524,0x4F284D1B,0xD8A31E9D,0xBA73B6E6,0xF399710D,0xBD8B1937,0x70FFE130,
	0x056DAA4A,0xDC509CA1,0x07358DFF,0xDF30A2DC,0x67E7349F,0x49532C31,0x2393EBAA,0xE54DF202,
	0x3A2C7EC9,0x98AB13EF,0x7FA52975,0x83E4792E,0x7485DA08,0x4A1823A8,0x77812011,0x8710BB89,
	0x9B4E0C68,0x64125D8E,0x5F174A0E,0x33EA50E7,0xA5E168B0,0x1BD9B944,0x6D7D8FE0,0xEE66B84C,
	0xF0DB530C,0xF8B06B72,0x97ED7DF8,0x126E0122,0x364BED23,0xA103B75C,0x3BC844FA,0xD0946501,
	0x4E2F70F1,0x79A6F413,0x60B9E977,0xC1582F10,0x759B286A,0xE723EEF5,0x8BAC4B39,0xB074B188,
	0xCC528E64,0x698700EE,0x44F9E5BB,0x7E336153,0xE2413AFD,0x91DCE2BE,0xFDCE9EC1,0xCAB2DE4F,
	0x46C5A486,0xA0D630DB,0x1FCD5FCA,0xEA110891,0x3F20C6F9,0xE8F1B25D,0x6EFD10C8,0x889027AF,
	0xF284AF3F,0x89EE9A61,0x58AF1421,0xE41B9269,0x260C6D71,0x5079D96E,0xD959E465,0x519CD72C,
	0x73B64F5A,0x40BE5535,0x78386CBC,0x0A1A02CF,0xDBC126B6,0xAD02BC8D,0x22A85BC5,0xA28ABEC3,
	0x5C643952,0xE35BC9AD,0xCBDACA63,0x4CA076A4,0x4B6121CB,0x9500BF7D,0x6F8E32BF,0xC06587E5,
	0x21FAEF46,0x9C2AD2F6,0x7691D4A2,0xB13E4687,0xC7460AD6,0xDDFE54D5,0x81F516F3,0xC60D7438,
	0xB9CB3BC7,0xC4770D94,0xF4571240,0x06862A50,0x30D343D3,0x5ACF52B2,0xACF4E68A,0x0FC2A59B,
	0xB70AEACD,0x53AA5E80,0xCF624E8F,0xF1214CEB,0x936072DF,0x62193F18,0xF5491CDA,0x5D476958,
	0xDA7A852D,0x5B053E12,0xC5A9F6D0,0xABD4A7D1,0xD25E6E82,0xA4D17314,0x2E148C4E,0x6B9F6399,
	0xBC26DB47,0x8296DDCE,0x3E71D616,0x350E4083,0x2063F503,0x167833F2,0x115CDC5E,0x4208E715,
	0x03A49B66,0x43A724BA,0xA3B71B8C,0x107584AE,0xC24AE0C6,0xB3FC6273,0x280F3795,0x1392C5D4,
	0xD5BAC762,0xB46B5A3B,0xC9480B8B,0xC39783FC,0x17F2935B,0x9DB482F4,0xA7E9CC09,0x553F4734,
	0x8DB5C3A3,0x7195EC7A,0xA8518A9A,0x0CE6CB2A,0x14D50976,0x99C077A5,0x012E1733,0x94EC3D7C,
	0x3D825805,0x0E80A920,0x1D39D1AB,0xFCD85126,0x3C7F3C79,0x7A43780B,0xB26815D9,0xAF1F7F1C,
	0xBB8D7C81,0xAAE5250F,0x34BC670A,0x1929C8D2,0xD6AE9FC0,0x1AE07506,0x416F3155,0x9EB38698,
	0x8F22CF29,0x04E8065F,0xE07CFBDE,0x2AEF90E8,0x6CAD049C,0x4DC3A8CC,0x597E3596,0x08562B92,
	0x52A21D6F,0xB6C9881D,0xFBD75784,0xF613FC32,0x54C6F757,0x66E2D57B,0xCD69FE9E,0x478CA13D,
	0x2F5F6428,0x8E55913C,0xF9091185,0x0089E8B3,0x1C6A48BD,0x3844946D,0x24CC8B6B,0x6524AC2B,
	0xD1F6A0F0,0x32980E51,0x8634CE17,0xED67417F,0x250BAEB9,0x84D2FD1A,0xEC6C4593,0x29D0C0B1,
	0xEBDF42A9,0x0D3DCD45,0x72BF963A,0x27F0B590,0x159D5978,0x3104ABD7,0x903B1F27,0x9F886A56,
	0x80540FA6,0x18F8AD1F,0xEF5A9870,0x85016FC2,0xC8362D41,0x6376C497,0xE1A15C67,0x6ABD806C,
	0x569AC1E2,0xFE5D1AF7,0x61CADF59,0xCE063874,0xD4F722DD,0x37DEC2EC,0xAE70BDEA,0x0B2D99B4,
	0x39B895FE,0x091E9DFB,0xA9150754,0x7D1D7A36,0x9A07B41E,0x5E8FE3B5,0xD34503A0,0xBE2BFAB7,
	0x5742D0A7,0x48DDBA25,0x7BE3604D,0x2D4C66E9,0xB831FFB8,0xF7BBA343,0x451697E4,0x2C4FD84B,
	0x96B17B00,0xB5C789E3,0xFFEBF9ED,0xD7C4B349,0xDE3281D8,0x689E4904,0xE683F32F,0x2B3CB0E1
};

const std::uint32_t System[256] =
{
	0x724FB987,0x4A3E70BE,0xCA549C50,0x34E263E1,0x2D5ED2FF,0x127F0E11,0x58A42B78,0x5F6D14AE,
	0x7E2F745D,0xC3450384,0xCFBB15DE,0xDF0A6D8A,0xEF2545F3,0x6D8919DB,0xBC413C94,0xCCB0A198,
	0xE42DBBD2,0x361C0B8C,0x8359731F,0x13D61E9F,0x7505F7CE,0x271D7957,0x429C0699,0xD84EC85F,
	0x953391DD,0xB25DE567,0xC1BA2F97,0x2309B605,0x69A134D1,0x14A092F2,0x681500EF,0xB90148A7,
	0x01AF398B,0x16FD5168,0x9E572161,0x0F7405E3,0x56AC576D,0xF275A349,0x1E8120C0,0x4BF64E3A,
	0x5A90E85E,0xD27BC4F1,0x3BD2FFB1,0xD6B40FDC,0x26EC61CF,0xF744AD3F,0xCDE7C548,0x8AFFE60A,
	0xE382CA47,0x87DA3E1B,0x8FA3DB36,0x5737C7E0,0xACD8CC17,0xD0CC3B66,0xD93D776B,0x37E5BE2B,
	0xD38A1129,0x037E81D0,0x15B15072,0xA6493052,0x35BCD4B9,0xC4538D32,0xEC66C1D5,0xA20DF513,
	0x5524EB75,0x92C10488,0xDA03D9FD,0x65168F4B,0x1902BA24,0x7439FA7D,0x1D8CB46F,0xFBC39389,
	0xC5DF6A58,0x89E8FB00,0x50DBE0A1,0xAAE98AF8,0x6A7C6C9C,0x7712D6EC,0x4030D0CD,0x6052B585,
	0x6132AA77,0xEB4A38C3,0x673AB1E6,0x1C3C07C6,0x91EA2C76,0x7A4C7EA0,0x10B3DCFC,0xBE7DF402,
	0x2817D87A,0x25632264,0xBD8D02B0,0xF6D0F8A8,0xB1ED3AF0,0xE6C4F1CA,0x99E028B5,0xE5D48674,
	0x09CF47B8,0x9D6EAF0E,0x0A721AFE,0xB6109E54,0x8D642344,0x9FEFC27C,0xF0CA520F,0x2C6BDA7E,
	0x2E9DB06A,0x97DEFC2E,0x53C5F0EE,0xAD4B8C60,0xE9F36696,0xA8C68907,0x70B70A20,0x3D9F82AA,
	0x7604A595,0x441A563B,0x39193D4A,0x33BF1DC7,0x31B283FB,0xA399F25B,0x642CE39E,0xF9E3B204,
	0x79A87534,0x5DBE2943,0x9813E93E,0x47864AD6,0xD420D1BF,0x24A6C986,0xFE386EF7,0xD1B65AB7,
	0x3A96BF2F,0x006FE1AB,0x22938E90,0x78FE7A40,0x5CE1319B,0x46F5EEF5,0xBB064BE4,0xB7271C22,
	0xC0225D21,0xFA145B10,0x7C58BC33,0xF84654C2,0xEEF4691E,0x021BEC16,0xE16C1737,0x1BCB2603,
	0x48A2954D,0xDD56A8FA,0xB8C8A48D,0x5277590B,0x1194E7A9,0x590F42B4,0x7B97C0D8,0x7142B714,
	0xAEDD6BC8,0xBA116212,0x6B0E642C,0xF42ABDC5,0x6E76AC81,0xBF348819,0xCB790C59,0xDC6718AD,
	0x80471230,0x84DC985C,0x2AEE32C1,0x4D35964F,0x0C6894AC,0x3EF2CDE5,0xB59B37A5,0x9BC9729D,
	0x186A41AF,0xEA98A970,0x21F8A291,0x5487E2C9,0xE05F3F42,0xA523B86E,0x8C1E4062,0xA962F6CB,
	0x0D4816E8,0x9A4DF92D,0x20439DCC,0xA0713645,0x43506FE9,0xC2EB4651,0xB4780D6C,0xAFC29B28,
	0x1FCE5FD4,0x9C7385D3,0xCE00E463,0x38CD997F,0x452933DA,0xC9F7DEBA,0x0840A093,0xDB287B41,
	0x90E48479,0x66FC6709,0x6C884C65,0x3FB56082,0xF5B87123,0xED367D1D,0x6F0C44F9,0x8270DD38,
	0x0E314F83,0x1AE69F35,0xD5A51FB3,0xA761A671,0x850B4DED,0x06AE0892,0x5EAA2A06,0xC7FA80F6,
	0xB0692D4E,0x81657F8F,0x948B0980,0xB3D97C01,0xFC80C3EA,0xFF9E53A4,0x30BD784C,0xF3AD970C,
	0xA12E9A31,0x04D37646,0x072655A3,0xE8D5F353,0x4CA98BDF,0x7391FE56,0x7D5BEDA6,0x2BD7650D,
	0x862B5C73,0x8B60A726,0x7F8ECB3C,0x517A49B6,0xD7B9CF5A,0x6308D5BC,0x0B3F68D7,0x62A7EA15,
	0xC65AFD3D,0xAB8525B2,0xA451B308,0xE7C7AB18,0x88F91369,0x1783279A,0x4F95DF2A,0x41F158BD,
	0xC8D1CEBB,0x325CD3E2,0xF1928739,0x9355AE8E,0x2FC05EC4,0x4E0735E7,0xDE3B10D9,0x8E18C61A,
	0xE29AEF25,0x4984D7A2,0x051F247B,0x29AB9055,0xFD2101F4,0x96FB2E1C,0x5BF04327,0x3C8F1BEB,
};
}
}

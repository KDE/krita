#include <cstdint>
#include <ctime>
#include <fstream>
#include <iostream>
#include <chrono>
#include <utility>
#include <sai.hpp>

#include "Benchmark.hpp"

class SaiTreeView : public sai::VirtualFileVisitor
{
public:
	SaiTreeView()
	:
	FolderDepth(0)
	{
	}
	~SaiTreeView()
	{
	}
	bool VisitFolderBegin(sai::VirtualFileEntry& Entry) override
	{
		PrintVirtualFileEntry(Entry);
		++FolderDepth;
		return true;
	}
	bool VisitFolderEnd(sai::VirtualFileEntry& /*Entry*/) override
	{
		--FolderDepth;
		return true;
	}
	bool VisitFile(sai::VirtualFileEntry& Entry) override
	{
		PrintVirtualFileEntry(Entry);
		return true;
	}
private:
	void PrintVirtualFileEntry(const sai::VirtualFileEntry& Entry) const
	{
		const std::time_t TimeStamp = Entry.GetTimeStamp();
		char TimeString[32];
		std::strftime(
			TimeString,
			32,
			"%D %R",
			std::localtime(&TimeStamp)
		);
		PrintNestedFolder();
		std::printf(
			"\u251C\u2500\u2500 [%12zu %s] %s\n",
			Entry.GetSize(),
			TimeString,
			Entry.GetName()
		);
	}
	void PrintNestedFolder() const
	{
		for( std::size_t i = 0; i < FolderDepth; ++i )
		{
			std::fputs(
				"\u2502   ",
				stdout
			);
		}
	}
	std::uint32_t FolderDepth;
};

const char* const Help =
"Show virtual file system tree of a user-created .sai files:\n"
"\t./Tree (filenames)\n"
"\tWunkolo - Wunkolo@gmail.com";

int main(int argc, char* argv[])
{
	if( argc < 2 )
	{
		std::puts(Help);
		return EXIT_FAILURE;
	}

	for( std::size_t i = 1; i < std::size_t(argc); ++i)
	{
		sai::Document CurDocument(argv[i]);

		if( !CurDocument.IsOpen() )
		{
			std::cout << "Error opening file for reading: " << argv[i] << std::endl;
			return EXIT_FAILURE;
		}

		const auto Bench = Benchmark<std::chrono::nanoseconds>::Run(
			[&CurDocument]() -> void
			{
				SaiTreeView TreeVisitor;
				CurDocument.IterateFileSystem(TreeVisitor);
			}
		);
		std::printf(
			"Iterated VFS of %s in %zu ns\n",
			argv[i],
			Bench.count()
		);
	}

	return EXIT_SUCCESS;
}

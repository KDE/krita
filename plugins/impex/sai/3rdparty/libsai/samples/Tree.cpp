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
        if (FolderDepth>0) {
            sai::Layer layerData = sai::Layer(Entry);
            PrintNestedFolder();
            PrintNestedFolder();
                std::cout  << " "
                           <<"LayerType: "<< LayerType(layerData.LayerType())
                           << ", ParentLayer: "<< layerData.ParentID()
                           << ", LayerID: "<< layerData.Identifier()
                         << ", LayerName: "<< layerData.LayerName()
                           << "\n";
            PrintNestedFolder();
            PrintNestedFolder();
                std::cout <<" Visibility:"
                          << layerData.IsVisible()
                          <<", Opacity:"
                          << layerData.Opacity()
                          << ", AlphaLock:"
                          << layerData.IsPreserveOpacity()
                          << ", Clipping:"
                          << layerData.IsClipping()
                          << "\n";
                PrintNestedFolder();
                PrintNestedFolder();
                std::cout << " Position: ["
                          << int(std::get<0>(layerData.Position()))
                          << ", "
                          << int(std::get<1>(layerData.Position()))
                          << "], Size: ["
                          << int(std::get<0>(layerData.Size()))
                          <<","
                          << int(std::get<1>(layerData.Size()))
                          << "] Blending Mode: "
                          << BlendingMode(layerData.Blending())
                          << "\n";



        }
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

    std::string LayerType(sai::LayerClass type) {
        std::string s = "";
        switch(type) {
        case sai::LayerClass::Layer:
            s = "RasterLayer";
            break;
        case sai::LayerClass::Linework:
            s = "LineworkLayer";
            break;
        case sai::LayerClass::Set:
            s = "Folder";
            break;
        case sai::LayerClass::Mask:
            s = "Mask";
            break;
        case sai::LayerClass::RootLayer:
            s = "RootLayer";
            break;
        default:
            s = "unknown";
            break;
        }
        return s;
    }
    std::string BlendingMode(sai::BlendingMode mode) {
        std::string s = "";
        switch (mode) {
        case sai::BlendingMode::Shade:
            s = "Shade";
            break;
        case sai::BlendingMode::Binary:
            s = "Binary";
            break;
        case sai::BlendingMode::Normal:
            s = "Normal";
            break;
        case sai::BlendingMode::Screen:
            s = "Screen";
            break;
        case sai::BlendingMode::Multiply:
            s = "Multiply";
            break;
        case sai::BlendingMode::LumiShade:
            s = "LumiShade";
            break;
        case sai::BlendingMode::Luminosity:
            s = "Luminosity";
            break;
        case sai::BlendingMode::PassThrough:
            s = "Passthrough";
            break;
        case sai::BlendingMode::Overlay:
            s = "Overlay";
            break;
        default:
            s = "unknown blending mode";
            std::cout << " unknown blending mode " << mode;
            printf("|%c%c%c%c|\n",
                reinterpret_cast<char*>(&mode)[3],
                reinterpret_cast<char*>(&mode)[2],
                reinterpret_cast<char*>(&mode)[1],
                reinterpret_cast<char*>(&mode)[0]
            );
            break;
        }
        return s;
    }
	std::uint32_t FolderDepth;
};

const char* const Help =
"Show virtual file system tree of a user-created .sai files:\n"
"\tDecrypt.exe (filename)\n"
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

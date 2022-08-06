#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <filesystem>
#include <string>

#define BSPVERSION 30
#define TOOLVERSION 2


typedef struct
{
	int fileofs, filelen;
} lump_t;

#define LUMP_ENTITIES 0
#define LUMP_PLANES 1
#define LUMP_TEXTURES 2
#define LUMP_VERTEXES 3
#define LUMP_VISIBILITY 4
#define LUMP_NODES 5
#define LUMP_TEXINFO 6
#define LUMP_FACES 7
#define LUMP_LIGHTING 8
#define LUMP_CLIPNODES 9
#define LUMP_LEAFS 10
#define LUMP_MARKSURFACES 11
#define LUMP_EDGES 12
#define LUMP_SURFEDGES 13
#define LUMP_MODELS 14

#define HEADER_LUMPS 15

typedef struct
{
	int version;
	lump_t lumps[HEADER_LUMPS];
} dheader_t;

const char* mapnames[] = {
	"canal1",
	"canal1b",
	"canal2",
	"canal3",
	"elevator",
	"hazard1",
	"hazard2",
	"hazard3",
	"hazard4",
	"hazard5",
	"hazard6",
	"maint",
	"outro",
	"power1",
	"power2",
	"security1",
	"security2",
	"teleport1",
	"teleport2",
	"tram1",
	"tram2",
	"tram3",
	"xen1",
	"xen2",
	"xen3",
	"xen4",
	"xen5",
	"xen6",
	"yard1",
	"yard2",
	"yard3",
	"yard3a",
	"yard3b",
	"yard4",
	"yard4a",
	"yard5",
	"yard5a"
};

bool bConverted = false;
int ConvertMap(const char* mapName);

void Bshift_ConvertMaps()
{
	if (bConverted)
		return;

	std::string gamedir = gEngfuncs.pfnGetGameDirectory();
	std::string wdir = std::filesystem::current_path().string();
	std::string dest = wdir + "\\" + gamedir + "_addon" + "\\maps";

	if (!std::filesystem::exists(wdir + "\\bshift\\maps\\ba_tram1.bsp")  
		|| std::filesystem::exists(wdir + "\\" + gamedir + "_addon\\maps\\ba_tram1.bsp"))
	{
		bConverted = true;
		return;
	}

	for (int i = 0; i < 36; i++)
	{
		std::string bshiftpath = wdir + "\\bshift\\maps\\ba_" + mapnames[i] + ".bsp";
		
		std::filesystem::copy(bshiftpath, dest);

		ConvertMap((dest + "\\ba_" + mapnames[i] + ".bsp").c_str());
	}

	bConverted = true;
}

int ConvertMap(const char *mapName)
{
	if (!mapName)
	{
		gEngfuncs.Con_Printf("Usage: BlueShiftBSPConverter <mapname>\n");
		return EXIT_FAILURE;
	}

	if (std::unique_ptr<FILE, decltype(std::fclose)*> file{std::fopen(mapName, "rb+"), &std::fclose}; file)
	{
		const auto start = std::ftell(file.get());

		if (start == -1L)
		{
			gEngfuncs.Con_Printf("Error getting start position in file \"%s\"\n", mapName);
			return EXIT_FAILURE;
		}

		dheader_t header{};

		if (std::fread(&header, sizeof(header), 1, file.get()) != 1)
		{
			gEngfuncs.Con_Printf("Error reading header from file \"%s\"\n", mapName);
			return EXIT_FAILURE;
		}

		// Swap the first 2 lumps
		std::swap(header.lumps[0], header.lumps[1]);

		if (std::fseek(file.get(), start, SEEK_SET))
		{
			gEngfuncs.Con_Printf("Error seeking to start in file \"%s\"\n", mapName);
			return EXIT_FAILURE;
		}

		if (std::fwrite(&header, sizeof(header), 1, file.get()) != 1)
		{
			gEngfuncs.Con_Printf("Error writing new header to file \"%s\"\n", mapName);
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
	else
	{
		gEngfuncs.Con_Printf("Error opening file \"%s\"\n", mapName);
		return EXIT_FAILURE;
	}
}
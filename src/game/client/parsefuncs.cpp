#include "hud.h"
#include "com_model.h"
#include "parsefuncs.h"

#define tokmatch(x) if(!stricmp(token,x))
#define loadfile(x) (char *)gEngfuncs.COM_LoadFile(x, 5, 0)
#define freefile	gEngfuncs.COM_FreeFile(afile)
#define parsefile 	pfile = gEngfuncs.COM_ParseFile(pfile, token)

viewmodelextrainf_t* g_curviewmodelextrainfo;
viewmodelextrainf_t g_viewmodelextrainfo[64];

void ParseFunc_ReadViewModelInfo()
{
	char tempname[64] = {"\0"};
	char token[64];
	char* pfile = nullptr, * afile = nullptr;
	pfile = afile = loadfile("scripts/modelconf.txt");

	int index = 0;

	if (!pfile)
		return;

	while (parsefile)
	{
		tokmatch("name")
		{
			parsefile;
			strcpy(tempname, token);
		}
		else if (strlen(tempname) > 0 && !stricmp(token,"{"))
		{
			while (parsefile)
			{
				tokmatch("}")
				{
					if (!stricmp(g_viewmodelextrainfo[index].name, tempname))
					{
						index++;
						strcpy(tempname, "\0");
					}
					break;
				}

				if (stricmp(g_viewmodelextrainfo[index].name, tempname))
				{
					strcpy(g_viewmodelextrainfo[index].name, tempname);
				}

				tokmatch("cambone")
				{
					parsefile;
					g_viewmodelextrainfo[index].cambone = atoi(token);
				}
				
				tokmatch("camscale")
				{
					parsefile;
					g_viewmodelextrainfo[index].camscale = atof(token);
				}

				tokmatch("bobscale")
				{
					parsefile;
					g_viewmodelextrainfo[index].bobscale = atof(token);
				}
			}
		}
	}

	freefile;
}

viewmodelextrainf_t* SearchViewModelInfo(const char* model)
{
	viewmodelextrainf_t* out = nullptr;
	for (int i = 0; i < 64; i++)
	{
		if (!(*g_viewmodelextrainfo[i].name))
			break;

		if (!stricmp(model, g_viewmodelextrainfo[i].name))
		{
			out = &g_viewmodelextrainfo[i];
			break;
		}
	}
	return out;
}

void SetCurViewModelInfo()
{
	cl_entity_t* view = gEngfuncs.GetViewModel();
	static char cachedname[64] = {"\0"};

	if (!view || !view->model)
		return;

	if (stricmp(cachedname, view->model->name))
	{
		g_curviewmodelextrainfo = SearchViewModelInfo(view->model->name);

		strcpy(cachedname, view->model->name);
	}
}
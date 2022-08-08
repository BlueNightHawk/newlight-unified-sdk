#include "cl_dll.h"
#include "PlatformHeaders.h"
#include <Psapi.h>
#include "FontAwesome.h"
#include "fs_aux.h"

#include "hl_imgui.h"
#include "CChapterSelectGUI.h"

extern SDL_Window* window;

void CChapterSelectGUI::Init()
{
	ParseChaptersFile();
	LoadImageTextures();
	CalcPages();
}

void CChapterSelectGUI::Draw()
{
	if (m_bMenuOpen == false || m_iNumChapters == 0)
		return;
 
	int RENDERED_WIDTH = 0, RENDERED_HEIGHT = 0;
	int WINDOW_POS_X = 0, WINDOW_POS_Y = 0;

	int start = 0, end = 3;

	GetStartEnd(start, end);

	SDL_GetWindowSize(window, &RENDERED_WIDTH, &RENDERED_HEIGHT);

	WINDOW_POS_X = (RENDERED_WIDTH - CHAPTER_SELECT_WINDOW_WIDTH) / 2;
	WINDOW_POS_Y = (RENDERED_HEIGHT - CHAPTER_SELECT_WINDOW_HEIGHT) / 2;

	ImGui::SetNextWindowPos(ImVec2(WINDOW_POS_X, WINDOW_POS_Y), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(CHAPTER_SELECT_WINDOW_WIDTH, CHAPTER_SELECT_WINDOW_HEIGHT), ImGuiCond_Once);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 10.6f));

	ImGui::Begin("Chapter Select", &m_bMenuOpen, WindowFlags());

	ImGui::SameLine(0.0f, 23.55f);

	ImGui::BeginTable("", 3, ImGuiSelectableFlags_SpanAllColumns);

	ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_IndentEnable);

	for (int i = start; i < end; i++)
	{
		ImGui::TableNextColumn();

		if (i >= m_iNumChapters)
			continue;

		ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Chapter %i", i+1);
		ImGui::Text(g_ChapterInfo[i].title);
		if (i == m_iSelectedChapter)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6, 0.60, 0.0, 1.0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60, 0.60, 0.0, 1.0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60, 0.60, 0.0, 1.0));
		}

		if (ImGui::ImageButton((void*)(intptr_t)g_ChapterInfo[i].texture, ImVec2(g_ChapterInfo[i].width, g_ChapterInfo[i].height)))
		{
			if (m_iSelectedChapter != i)
			{
				m_iSelectedChapter = i;
				continue;
			}
		}

		if (i == m_iSelectedChapter)
		{
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
	}

	ImGui::EndTable();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::NewLine();
	ImGui::SameLine(0.0f, 23.0f);
	if (m_iCurrentPage > 0)
	{
		if (ButtonText("Back"))
		{
			m_iCurrentPage--;
		}
	}
	ImGui::SameLine(CHAPTER_SELECT_WINDOW_WIDTH - (ImGui::CalcTextSize("Next").x + 71));
	if (m_iCurrentPage < (m_iNumPages-1))
	{
		if (ButtonText("Next"))
		{
			m_iCurrentPage++;
		}
	}
	else
	{
		ImGui::NewLine();
	}

	ImGui::NewLine();

	ImGui::Separator();

	ImGui::PopStyleVar();

	ImGui::NewLine();
	ImGui::NewLine();

	ImGui::SameLine(CHAPTER_SELECT_WINDOW_WIDTH - (ImGui::CalcTextSize("Cancel").x + 65));
	ImGui::SetCursorPosY(CHAPTER_SELECT_WINDOW_HEIGHT - 37);

	if (ButtonText("Cancel"))
	{
		m_bMenuOpen = false;
	}

	ImGui::SameLine(CHAPTER_SELECT_WINDOW_WIDTH - (ImGui::CalcTextSize("Start New Game").x + 130));
	ImGui::SetCursorPosY(CHAPTER_SELECT_WINDOW_HEIGHT - 37);
	if (m_iSelectedChapter > -1)
	{
		if (ButtonText("Start New Game"))
		{
			char cmd[64];
			sprintf(cmd, "map %s \n", g_ChapterInfo[m_iSelectedChapter].mapname);

			gEngfuncs.pfnClientCmd(cmd);
		}
	}

	ImGui::End();
}

void CChapterSelectGUI::CalcPages()
{
	int remainder = 0;
	for (int i = 0; i < m_iNumChapters; i++)
	{
		remainder = ((i + 1) % 3);

		if (remainder == 0)
			m_iNumPages++;
	}

	if (remainder != 0)
		m_iNumPages++;
}

void CChapterSelectGUI::GetStartEnd(int& start, int& end)
{
	if (m_iCurrentPage == 0)
	{
		start = 0;
		end = 3;
		return;
	}

	start = 3*m_iCurrentPage;
	end = start + 3;
	
	if (end >= m_iNumChapters)
		end = m_iNumChapters;
}



void CChapterSelectGUI::ParseChaptersFile()
{
	char *afile, *pfile;
	char token[128];
	int index = 0;

	afile = pfile = (char*)gEngfuncs.COM_LoadFile("resource/chapters.txt", 5, nullptr);

	if (!afile || !pfile)
		return;

	while (pfile = gEngfuncs.COM_ParseFile(pfile, token))
	{
		if (!stricmp(token, "title"))
		{
			pfile = gEngfuncs.COM_ParseFile(pfile, token);
			strcpy(g_ChapterInfo[index].title, token);
			pfile = gEngfuncs.COM_ParseFile(pfile, token);
			strcpy(g_ChapterInfo[index].mapname, token);
			index++;
		}
	}

	gEngfuncs.COM_FreeFile(afile);
	afile = pfile = nullptr;
}

void CChapterSelectGUI::LoadImageTextures()
{
	const char* chapterfolder = "chaptericons";
	const char* prefix = "ch";
	const char* ext = ".png";
	const char* gamedir = gEngfuncs.pfnGetGameDirectory();
	char fullpath[128];

	for (int i = 0; i < MAX_CHAPTERS; i++)
	{
		sprintf(fullpath, ".\\%s\\%s\\%i%s", gamedir, chapterfolder, i + 1, ext);
		if (LoadTextureFromFile(fullpath, &g_ChapterInfo[i].texture, &g_ChapterInfo[i].width, &g_ChapterInfo[i].height))
			m_iNumChapters++;
		else
			break;
	}
}

void CChapterSelectGUI::DeleteImageTextures()
{
	for (int i = 0; i < m_iNumChapters; i++)
	{
		glDeleteTextures(1, &g_ChapterInfo[i].texture);
	}
}

bool CChapterSelectGUI::ButtonText(const char* text)
{
	static char out[512] = {"\0"};
	strcpy(out, text);

	for (int i = strlen(text); i < 15; i++)
	{
		out[i] = ' ';
	}

	return ImGui::Button(out);
}

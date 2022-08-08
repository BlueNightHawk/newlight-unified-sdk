#pragma once

#define MAX_CHAPTERS 32
#define CHAPTER_SELECT_WINDOW_WIDTH 895
#define CHAPTER_SELECT_WINDOW_HEIGHT 340

typedef struct chapterinfo_s
{
	// Texture info
	GLuint texture = 0;
	int width = 0;
	int height = 0;

	// Chapter related info
	char title[64] = {"\0"};
	char mapname[64] = {"\0"};
} chapterinfo_t;

class CChapterSelectGUI
{
public:
	void Init();
	void Draw();
	void DeleteImageTextures();

private:
	void ParseChaptersFile();
	void LoadImageTextures();

	void CalcPages();

	bool ButtonText(const char* text);

	void GetStartEnd(int& start, int& end);

	const int WindowFlags() {
		return ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
			ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings;
	}

	int m_iCurrentPage = 0;
	int m_iNumPages = 0;
	int m_iNumChapters = 0;
	int m_iSelectedChapter = -1;

	bool m_bMenuOpen = true;
};

inline CChapterSelectGUI g_ChapterSelectGUI;
inline chapterinfo_t g_ChapterInfo[MAX_CHAPTERS];
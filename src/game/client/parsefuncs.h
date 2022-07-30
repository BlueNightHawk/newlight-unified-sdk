#pragma once

typedef struct viewmodelextrainf_s {
	char name[64]{"\0"};
	int cambone{0};
	float camscale{0.0f};
	float bobscale{0.0f};
} viewmodelextrainf_t;

void ParseFunc_ReadViewModelInfo();
viewmodelextrainf_t* SearchViewModelInfo(const char* model);
void SetCurViewModelInfo();

extern viewmodelextrainf_t *g_curviewmodelextrainfo;
extern viewmodelextrainf_t g_viewmodelextrainfo[64];
#pragma once

void InitializeFBX();
void DestroyFBX();

bool ImportFBX(const wchar_t* fileDirectory, void** p);
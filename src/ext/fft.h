#pragma once
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////

bool FFT_Open(bool CapturePlaybackDevices, const char* CaptureDeviceSearchString);
void FFT_EnumerateDevices();
void FFT_GetFFT(float* _samples);
void FFT_Close();

//////////////////////////////////////////////////////////////////////////

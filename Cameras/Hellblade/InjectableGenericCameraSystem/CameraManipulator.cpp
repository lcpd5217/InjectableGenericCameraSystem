////////////////////////////////////////////////////////////////////////////////////////////////////////
// Part of Injectable Generic Camera System
// Copyright(c) 2017, Frans Bouma
// All rights reserved.
// https://github.com/FransBouma/InjectableGenericCameraSystem
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
//  * Redistributions of source code must retain the above copyright notice, this
//	  list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and / or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CameraManipulator.h"
#include "GameConstants.h"
#include "InterceptorHelper.h"
#include "Globals.h"
#include "OverlayConsole.h"

using namespace DirectX;
using namespace std;

extern "C" {
	LPBYTE g_cameraStructAddress = nullptr;
	LPBYTE g_timestopStructAddress = nullptr;
}

namespace IGCS::GameSpecific::CameraManipulator
{
	static float _originalCoords[3];
	static float _originalAngles[3];
	static float _currentCameraCoords[3];
	
	// Resets the FOV to the one it got when we enabled the camera
	void resetFoV()
	{
		if (nullptr == g_cameraStructAddress)
		{
			return;
		}
		// there are two offsets, one is the regular game fov, the other is the photomode offset. If the photomode offset isn't bigger than 0, we write
		// the regular fov offset, otherwise we'll write the photomode offset
		float* fovInMemory = reinterpret_cast<float*>(g_cameraStructAddress + FOVPM_IN_STRUCT_OFFSET);
		if (*fovInMemory > 0.0f)
		{
			*fovInMemory = DEFAULT_FOV_DEGREES;
		}
		else
		{
			fovInMemory = reinterpret_cast<float*>(g_cameraStructAddress + FOV_IN_STRUCT_OFFSET);
			*fovInMemory = DEFAULT_FOV_DEGREES;
		}
		g_fovValue = DEFAULT_FOV_DEGREES;
	}


	// changes the FoV with the specified amount
	void changeFoV(float amount)
	{
		if (nullptr == g_cameraStructAddress)
		{
			return;
		}
		// there are two offsets, one is the regular game fov, the other is the photomode offset. If the photomode offset isn't bigger than 0, we write
		// the regular fov offset, otherwise we'll write the photomode offset
		float* fovInMemory = reinterpret_cast<float*>(g_cameraStructAddress + FOVPM_IN_STRUCT_OFFSET);
		if (*fovInMemory > 0.0f)
		{
			*fovInMemory += amount;
		}
		else
		{
			fovInMemory = reinterpret_cast<float*>(g_cameraStructAddress + FOV_IN_STRUCT_OFFSET);
			*fovInMemory += amount;
		}
		g_fovValue += amount;
	}


	XMFLOAT3 getCurrentCameraCoords()
	{
		return XMFLOAT3(_currentCameraCoords[0], _currentCameraCoords[1], _currentCameraCoords[2]);
	}
	

	// newCoords are the new coordinates for the camera in worldspace.
	// yaw, pitch, roll are the angles used for the rotation as they're now defined in the camera. These angles have to be written as floats (in degrees)
	// pitch, yaw, roll (they store it as 'x rotation', 'y rotation', 'z rotation'). 
	void writeNewCameraValuesToGameData(XMFLOAT3 newCoords, float pitch, float yaw, float roll)
	{
		if (nullptr == g_cameraStructAddress)
		{
			return;
		}
		// this camera is stored as 3 coords (floats) and 3 angles (floats). 
		float* coordsInMemory = reinterpret_cast<float*>(g_cameraStructAddress + COORDS_IN_STRUCT_OFFSET);
		coordsInMemory[0] = newCoords.x;
		coordsInMemory[1] = newCoords.y;
		coordsInMemory[2] = newCoords.z;
		float* anglesInMemory = reinterpret_cast<float*>(g_cameraStructAddress + ANGLES_IN_STRUCT_OFFSET);
		anglesInMemory[0] = pitch;
		anglesInMemory[1] = yaw;
		anglesInMemory[2] = roll;

		_currentCameraCoords[0] = newCoords.x;
		_currentCameraCoords[1] = newCoords.y;
		_currentCameraCoords[2] = newCoords.z;
	}


	bool isCameraFound()
	{
		return nullptr != g_cameraStructAddress;
	}


	void displayCameraStructAddress()
	{
		OverlayConsole::instance().logDebug("Camera struct address: %p", (void*)g_cameraStructAddress);
	}
	

	// should restore the camera values in the camera structures to the cached values. This assures the free camera is always enabled at the original camera location.
	void restoreOriginalCameraValues()
	{
		float* coordsInMemory = reinterpret_cast<float*>(g_cameraStructAddress+ COORDS_IN_STRUCT_OFFSET);
		memcpy(coordsInMemory, _originalCoords, 3 * sizeof(float));
		float* anglesInMemory = reinterpret_cast<float*>(g_cameraStructAddress + ANGLES_IN_STRUCT_OFFSET);
		memcpy(anglesInMemory, _originalAngles, 3 * sizeof(float));
	}


	void cacheOriginalCameraValues()
	{
		float* coordsInMemory = reinterpret_cast<float*>(g_cameraStructAddress + COORDS_IN_STRUCT_OFFSET);
		memcpy(_originalCoords, coordsInMemory, 3 * sizeof(float));
		_currentCameraCoords[0] = _originalCoords[0];		// x
		_currentCameraCoords[1] = _originalCoords[1];		// y
		_currentCameraCoords[2] = _originalCoords[2];		// z
		float* anglesInMemory = reinterpret_cast<float*>(g_cameraStructAddress + ANGLES_IN_STRUCT_OFFSET);
		memcpy(_originalAngles, anglesInMemory, 3 * sizeof(float));
	}
}
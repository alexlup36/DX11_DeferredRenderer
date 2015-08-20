#ifndef DXUTIL_H
#define DXUTIL_H

#include <d3d11.h>

#include "dxerr.h"

#ifndef HR
#ifdef  _DEBUG
#define  HR(x) \
{ \
	HRESULT hr = x; \
	if(FAILED(hr)) \
	{\
		DXTraceW(__FILEW__, __LINE__, hr, L#x, TRUE); \
	}\
}
#else  // !_DEBUG
#define  HR(x) x;
#endif  // _DEBUG
#endif // !HR

// ----------------------------------------------------------------------------

float Random(float maxFloat);
float Random(float minFloat, float maxFloat);

// ----------------------------------------------------------------------------

HRESULT CompileShader(_In_ LPCWSTR srcFile,
	_In_ LPCSTR entryPoint,
	_In_ LPCSTR profile,
	_Outptr_ ID3DBlob** blob);

// ----------------------------------------------------------------------------

#endif // DXUTIL_H
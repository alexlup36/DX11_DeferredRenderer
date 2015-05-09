#ifndef DXUTIL_H
#define DXUTIL_H

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



#endif // DXUTIL_H
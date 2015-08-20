#ifndef MESH_H
#define MESH_H

// ----------------------------------------------------------------------------

#include <vector>
#include <fstream>
#include <istream>

#include "DXUtil.h"
#include "SimpleMath.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// ----------------------------------------------------------------------------

struct SurfaceMaterial
{
	std::string	MaterialName;
	Vector4			DiffuseColor;
	int				TextureArrayIndex;
	bool			HasTexture;
	bool			Transparent;
};

// ----------------------------------------------------------------------------

struct FaceType
{
	int iVertexPosIndex1, iVertexPosIndex2, iVertexPosIndex3;
	int iVertexTextureCIndex1, iVertexTextureCIndex2, iVertexTextureCIndex3;
	int iVertexNormalIndex1, iVertexNormalIndex2, iVertexNormalIndex3;
};

// ----------------------------------------------------------------------------

class Mesh
{
public:
	Mesh();

	void CleanUp();

	bool LoadOBJ(std::string fileName,
		bool bRHCoordinateSystem);

private:

	// Mesh data
	ID3D11Buffer*								m_pVertexBuffer		= nullptr;
	ID3D11Buffer*								m_pIndexBuffer		= nullptr;

	// World matrix
	Matrix										m_mWorld;
};

// ----------------------------------------------------------------------------

#endif // MESH_H
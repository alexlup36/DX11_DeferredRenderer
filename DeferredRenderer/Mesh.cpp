#include "Mesh.h"

#include <iostream>
#include <stdio.h>
#include <string>

// ----------------------------------------------------------------------------

Mesh::Mesh()
{
}

// ----------------------------------------------------------------------------

void Mesh::CleanUp()
{
	if (m_pVertexBuffer != nullptr)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}

	if (m_pIndexBuffer != nullptr)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}
}

// ----------------------------------------------------------------------------

bool Mesh::LoadOBJ(std::string fileName,
	bool bRHCoordinateSystem)
{
	HRESULT hr = 0;

	// Open the input file
	std::ifstream inputFile(fileName.c_str(), std::ifstream::in);

	// Vertex data
	std::vector<Vector3> vertexPositions;
	std::vector<Vector3> vertexNormals;
	std::vector<Vector2> vertexTexCoordinates;

	// Face data
	std::vector<FaceType> faceList;

	char currentChar;
	
	// Check the file
	if (inputFile)
	{
		while (inputFile)
		{
			// Get next character
			inputFile.get(currentChar);

			// Check the character type
			switch (currentChar)
			{
				// Comment -> skip line
				case '#':
				case 'm':
				case 'u':
				case 's':
				{
					while (inputFile.get() != '\n');
					break;
				}
				
				// Vertex
				case 'v':
				{
					inputFile.get(currentChar);

					if (currentChar == ' ') // Vertex position
					{
						float x, y, z;
						inputFile >> x >> y >> z;
						if (bRHCoordinateSystem)
						{
							vertexPositions.push_back(Vector3(x, y, z * -1.0f));
						}
						else
						{
							vertexPositions.push_back(Vector3(x, y, z));
						}
					}
					if (currentChar == 't') // Vertex texture coordinate
					{
						float u, v;
						inputFile >> u >> v;
						if (bRHCoordinateSystem)
						{
							vertexTexCoordinates.push_back(Vector2(u, v * -1.0f));
						}
						else
						{
							vertexTexCoordinates.push_back(Vector2(u, v));
						}
					}
					if (currentChar == 'n') // Vertex normal
					{
						float nx, ny, nz;
						inputFile >> nx >> ny >> nz;
						if (bRHCoordinateSystem)
						{
							vertexNormals.push_back(Vector3(nx, ny, nz * -1.0f));
						}
						else
						{
							vertexNormals.push_back(Vector3(nx, ny, nz));
						}
					}

					break;
				}

				// Load face
				case 'f':
				{
					inputFile.get(currentChar);

					if (currentChar == ' ')
					{
						// Fill the current face
						FaceType face;
						ZeroMemory(&face, sizeof(FaceType));

						// Read the face data
						inputFile >> face.iVertexPosIndex1 >> currentChar >> face.iVertexTextureCIndex1 >> currentChar >> face.iVertexNormalIndex1
							>> face.iVertexPosIndex2 >> currentChar >> face.iVertexTextureCIndex2 >> currentChar >> face.iVertexNormalIndex3
							>> face.iVertexPosIndex3 >> currentChar >> face.iVertexTextureCIndex3 >> currentChar >> face.iVertexNormalIndex3;

						// Add a new face
						faceList.push_back(face);
					}
				}

				default:
				{
					break;
				}
			}
		}

		// Close the file
		inputFile.close();
	}
	else
	{
		MessageBox(nullptr, L"Could not open the model file.", L"Error", MB_OK);

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------------
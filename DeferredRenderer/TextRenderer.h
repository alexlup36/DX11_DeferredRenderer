#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <d3d11.h>

#include "SpriteBatch.h"
#include "SpriteFont.h"

#include "SimpleMath.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class TextRenderer
{
public:
	static TextRenderer& GetInstance()
	{
		static TextRenderer instance;
		return instance;
	}

	void Initialize(ID3D11DeviceContext* deviceContext, ID3D11Device* device);
	void RenderText(int iFPS, double dFrameTime);

private:
	TextRenderer() {}

	TextRenderer(TextRenderer const&)		= delete;
	void operator=(TextRenderer const&)		= delete;

	// Private members

	// Sprite batch
	std::unique_ptr<DirectX::SpriteBatch> m_SpriteBatch;
	// Sprite font
	std::unique_ptr<DirectX::SpriteFont> m_SpriteFont;
};

#endif // TEXTRENDERER_H
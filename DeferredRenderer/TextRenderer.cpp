#include "TextRenderer.h"

#include <string>

// ----------------------------------------------------------------------------

void TextRenderer::Initialize(ID3D11DeviceContext* deviceContext, ID3D11Device* device)
{
	// Initialize the sprite batch
	m_SpriteBatch.reset(new DirectX::SpriteBatch(deviceContext));
	// Initialize the sprite font
	m_SpriteFont.reset(new DirectX::SpriteFont(device, L"ArialFont.spritefont"));
}

// ----------------------------------------------------------------------------

void TextRenderer::RenderText(int iFPS, double dFrameTime)
{
	m_SpriteBatch->Begin();

	std::wstring fpsValue = L"FPS: " + std::to_wstring(iFPS) + L"\n" + L"Frame time: " + std::to_wstring(dFrameTime);
	m_SpriteFont->DrawString(m_SpriteBatch.get(), fpsValue.c_str(), Vector2(10, 10));

	m_SpriteBatch->End();
}

// ----------------------------------------------------------------------------
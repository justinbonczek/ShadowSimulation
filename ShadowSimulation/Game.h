//
// Reusable class for setting up a DirectX11 Application (particularly, a game!)
// Justin Bonczek
//

#ifndef GAME_H
#define GAME_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <Windows.h>
#include <string>

#define ReleaseMacro(x) { if(x){ x->Release(); x = 0; } }

class Game
{
public:
	Game(HINSTANCE hInstance);
	virtual ~Game(void);

	/// <summary>Handles Windows messages</summary>
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/// <summary>Game Loop</summary>
	int Run();

	/// <summary>Sets up the window and DirectX 11</summary>
	virtual bool Initialize();

	/// <summary>When the window is resized, fixes the back buffer, viewport, render target view, etc.</summary>
	virtual void OnResize();

	/// <summary>Pure virtual function to update game logic</summary>
	virtual void Update(float dt) = 0;

	/// <summary>Pure virtual function to render game objects</summary>
	virtual void Draw() = 0;

	/// <summary>Calculates the aspect ratio of the window</summary>
	float AspectRatio()const;
protected:
	/// <summary>Sets up the window</summary>
	bool InitializeWindow();

	/// <summary>Sets up DirectX 11</summary>
	bool InitializeDirect3D();

	float deltaTime;

	// Handlers to our instance and window
	HINSTANCE hInstance;
	HWND	  hWnd;

	ID3D11Device*			dev;
	ID3D11DeviceContext*	devCon;
	IDXGISwapChain*			swapChain;
	ID3D11Texture2D*		depthStencilBuffer;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	D3D11_VIEWPORT			viewport;
	D3D_FEATURE_LEVEL		featureLevel;

	UINT windowWidth;
	UINT windowHeight;
	std::wstring windowTitle;
};


#endif
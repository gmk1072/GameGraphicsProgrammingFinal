#pragma once
#include <Windows.h>
#include <string>

class DXWindow
{
public:
	DXWindow(
		HINSTANCE hInstance,		// The application's handle
		char* titleBarText,			// Text for the window's title bar
		unsigned int windowWidth,	// Width of the window's client area
		unsigned int windowHeight,	// Height of the window's client area
		bool debugTitleBarStats		// Show extra stats (fps) in title bar?
	);
	~DXWindow();


	// Static requirements for OS-level message processing
	static DXWindow* DXWindowInstance;
	static LRESULT CALLBACK WindowProc(
		HWND hWnd,		// Window handle
		UINT uMsg,		// Message
		WPARAM wParam,	// Message's first parameter
		LPARAM lParam	// Message's second parameter
	);

	// Internal method for message handling
	LRESULT ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Initialization and game-loop related methods
	HRESULT InitWindow();
	HRESULT Run();
	void Quit();

	// Pure virtual methods for setup and game functionality
	virtual void Init() = 0;
	virtual void Update(float deltaTime, float totalTime) = 0;
	virtual void Draw(float deltaTime, float totalTime) = 0;
	virtual void OnResize(unsigned int width, unsigned int height) = 0;

	// Convenience methods for handling mouse input, since we
	// can easily grab mouse input from OS-level messages
	virtual void OnMouseDown(WPARAM buttonState, int x, int y) { }
	virtual void OnMouseUp(WPARAM buttonState, int x, int y) { }
	virtual void OnMouseMove(WPARAM buttonState, int x, int y) { }
	virtual void OnMouseWheel(float wheelDelta, int x, int y) { }

	// Getters
	unsigned const int& GetWidth() const;
	unsigned const int& GetHeight() const;
	const POINTS& GetWindowLocation() const;
	const HWND& GetWindowReference() const;

protected:
	HINSTANCE	hInstance;		// The handle to the application
	HWND		hWnd;			// The handle to the window itself
	std::string titleBarText;	// Custom text in window's title bar
	bool		titleBarStats;	// Show extra stats in title bar?

	// Helper function for allocating a console window
	void CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns);
private:
	// Rectangle location of window in screen space
	POINTS windowLocation;

	// Size of the window's client area
	unsigned int width;
	unsigned int height;

	// Timing related data
	double perfCounterSeconds;
	float totalTime;
	float deltaTime;
	__int64 startTime;
	__int64 currentTime;
	__int64 previousTime;

	// FPS calculation
	int fpsFrameCount;
	float fpsTimeElapsed;

	void UpdateTimer();			// Updates the timer for this frame
	void UpdateTitleBarStats();	// Puts debug info in the title bar
};


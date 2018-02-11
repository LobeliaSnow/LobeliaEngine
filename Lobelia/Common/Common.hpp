#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>
#include <future>
#include <queue>
#include <list>
#include <stack>
#include <shlwapi.h>
#include <sstream>
#include <iostream>
#include <filesystem>
//
//#include <mfapi.h>
//#include <mfidl.h>
//#include <mfreadwrite.h>
//#include <mferror.h>
//#include <evr.h>

//#include <mfapi.h>
//#include <mfidl.h>
//#include <mfreadwrite.h>
//#include <mferror.h>
//#include <evr.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"shlwapi.lib")

//#pragma comment(lib, "mf.lib")
//#pragma comment(lib, "mfplat.lib")
//#pragma comment(lib, "mfuuid.lib")
//#pragma comment(lib, "mfreadwrite.lib")
//#pragma comment(lib,"strmiids.lib")

#include "Define/Define.hpp"
#ifdef USE_IMGUI_AND_CONSOLE
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx11.h"
LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
//#include "Literal/Literal.hpp"

#include "Math/Math.hpp"
#include "Timer/Timer.h"
#include "Window/Window.h"
#include "Utility/Utility.hpp"


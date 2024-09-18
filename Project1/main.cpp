#include <Windows.h>
#include <tchar.h>
#include <vector>

// DirectXやDSGIを制御するために必要なヘッダーのインクルード
#include <d3d12.h>
#include <dxgi1_6.h>

// ライブラリのリンク
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#ifdef _DEBUG
#include <iostream>
#endif 

using namespace std;

// @brief コンソール画面にフォーマット付き文字列を表示
// @param formatフォーマット（%dとおか%fとかの）
// @param 可変長引数
// @remarks この関数はデバッグ用です。デバッグ時にしか動作しません
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

// 面倒だけど書かなければいけない関数
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);	// OSに対して「もうこのアプリは終わると伝える」
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);	// 既定の処理を行う
}

// ウィンドウ作成
void MakeWindow(WNDCLASSEX& w)
{
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	// コールバック関数の指定
	w.lpszClassName = _T("DX12Sample");			// アプリケーションクラス名（適当でよい）
	w.hInstance = GetModuleHandle(nullptr);		// ハンドルの取得

	RegisterClassEx(&w);	// アプリケーションクラス(ウィンドウクラスの指定をOSに伝える)

	// 任意のウィンドウサイズを設定
	int window_width = 1600;
	int window_height = 900;

	RECT wrc = { 0,0,window_width,window_height };	// ウィンドウサイズを決める

	// 関数を使ってウィンドウのサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(w.lpszClassName,	// クラス名指定
		_T("DX12テスト"),			// タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	// タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,			// 表示x座標はOSにお任せ
		CW_USEDEFAULT,			// 表示y座標はOSにお任せ
		wrc.right - wrc.left,	// ウィンドウ幅
		wrc.bottom - wrc.top,	// ウィンドウ高
		nullptr,				// 親ウィンドウハンドル
		nullptr,				// メニューハンドル
		w.hInstance,			// 呼び出しアプリケーションハンドル
		nullptr);

	// ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);
}

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif

	// ウィンドウクラスの生成＆登録
	WNDCLASSEX w = {};

	// ウィンドウ作成
	MakeWindow(w);

	// DirectXの初期化


	/*DirectX基本部分の初期化*/

	// 受け皿となる変数の用意
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	// コマンドリストとコマンドアロケーターの宣言
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;

	// グラフィックスボードが複数刺さっている場合の処理

	// DXGIFactoryオブジェクトを生成する
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));	// DXGIFactoryが_dxgiFactoryに入る

	// アダプターの列挙用
	std::vector <IDXGIAdapter*> adapters;

	// ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter* tempAdapter = nullptr;

	// アダプターを配列に保存していく
	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tempAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(tempAdapter);
	}

	// 特定のアダプターを探し出す
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);	// アダプターの説明オブジェクトを取得

		std::wstring strDesc = adesc.Description;	// アダプターの名前を取得

		// 探したいアダプターの名前を確認
		if (strDesc.find(L"NVIDIA") != std::string::npos)	// このコードはNVIDIAを使っているから（配布や他のPCだとあまりよくない）
		{
			tempAdapter = adpt;
			break;
		}
	}

	// フィーチャーレベル構造体
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	// Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL featurelevel;

	// 使用できるバージョンを見つけてデバイスオブジェクトを生成する
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tempAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featurelevel == lv;
			break;	// 生成可能なバージョンが見つかったらループを打ち切り
		}
	}

	


	MSG msg = {};
	// アプリケーションが終了しないための無限ループ
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	











	// もうクラスは使わないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);

	//DebugOutputFormatString("Show window test.");
	//getchar();
	return 0;
}

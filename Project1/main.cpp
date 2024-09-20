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
	vprintf(format, valist);
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

// デバッグレイヤーを有効化する関数
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));

	debugLayer->EnableDebugLayer();	// デバッグレイヤーを有効化する
	debugLayer->Release();	// 有効化したらインターフェイスを開放する
}

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif

	// 任意のウィンドウサイズを設定
	int window_width = 1600;
	int window_height = 900;

	// ウィンドウクラスの生成＆登録
	WNDCLASSEX w = {};

	// ウィンドウ作成
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	// コールバック関数の指定
	w.lpszClassName = _T("DX12Sample");			// アプリケーションクラス名（適当でよい）
	w.hInstance = GetModuleHandle(nullptr);		// ハンドルの取得

	RegisterClassEx(&w);	// アプリケーションクラス(ウィンドウクラスの指定をOSに伝える)

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

	// デバッグレイヤーを有効にする
#ifdef _DEBUG
	// デバッグレイヤーをオン
	EnableDebugLayer();
#endif 

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
#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));	// DXGIFactoryが_dxgiFactoryに入る
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));	// DXGIFactoryが_dxgiFactoryに入る
#endif

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
			featurelevel = lv;
			break;	// 生成可能なバージョンが見つかったらループを打ち切り
		}
	}

	// リストとアロケーターの生成
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));

	// コマンドキューの宣言
	ID3D12CommandQueue* _cmdQueue = nullptr;

	// キューの設定のための構造体
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	// タイムアウトなし
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// アダプターを一つしか使わないときは０でいい	
	cmdQueueDesc.NodeMask = 0;

	// プライオリティは特になし
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	// コマンドリストと合わせる
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// キューの生成
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	// スワップチェーンの構造体
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	// 適用する設定
	swapchainDesc.Width = window_width;					// 画面解像度(幅)
	swapchainDesc.Height = window_height;				// 画面解像度(高)
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// ピクセルフォーマット
	swapchainDesc.Stereo = false;						// ステレオ表示プラグ(3Dディスプレイのステレオモード)
	swapchainDesc.SampleDesc.Count = 1;					// マルチサンプルの指定
	swapchainDesc.SampleDesc.Quality = 0;				// マルチサンプルの指定
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;	// DXGI_USAGE_BACK_BUFFERでよい
	swapchainDesc.BufferCount = 2;						// ダブルバッファーなら２でよい

	// バックバッファーは伸び縮み可能
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	// フリップ後は速やかに破棄
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Alphaモードの指定は特になし
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// ウィンドウ⇔フルスクリーン切り替え可能
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// スワップチェーンの生成
	result = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&_swapchain);

	// ディスクプリタヒープの構造体
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// レンダーターゲットビューなのでRTV
	heapDesc.NodeMask = 0;	// 複数のGPUを使用する場合、識別するためのもの
	heapDesc.NumDescriptors = 2;	//表裏の二つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// 特に指定なし

	// ディスクプリタヒープの生成
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	// スワップチェーンのメモリと紐付ける
	
	// 構造体の宣言
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	
	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	for(int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		// スワップチェーン上のバックバッファーをbuckBufferを取得
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));

		// ディスクプリタが必要なサイズはそれぞれ違うため逐一調べてサイズ分ずらす
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	}

	// コマンドアロケーターの命令オブジェクトをクリアする
	result = _cmdAllocator->Reset();

	MSG msg = {};
	// アプリケーションが終了しないための無限ループ
	while (true)
	{
		// バックバッファーを指すインデックスを取得
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		// 上のインデックスで示されたレンダーターゲットビューをこれから使うものとしてセットする
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		
		// バリアの構造体を作成
		D3D12_RESOURCE_BARRIER BarrierDesc = {};

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	// 遷移
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	// 特に指定なし
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];		// バックバッファーリソース
		BarrierDesc.Transition.Subresource = 0;

		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;	// 直前はPRESENT状態
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	// 今からレンダーターゲット状態

		_cmdList->ResourceBarrier(1, &BarrierDesc);	// バリア指定実行

		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		// レンダーターゲットを特定の色でクリアする

		// 画面をクリアする色を指定
		float clearColor[] = { 0.0f,0.0f,1.0f,1.0f };	// 黄色
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);	// 第二、第三引数はクリアする範囲

		// ためていた命令を実行する

		// 命令のクローズ(必須)
		_cmdList->Close();

		// クローズした後にコマンドリストの実行
		ID3D12CommandList* cmdlists[] = { _cmdList };

		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		// フェンスの作成
		ID3D12Fence* _fence = nullptr;
		UINT64 _fenceVal = 0;

		result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

		// GPUの処理が終わるまで
		_cmdQueue->Signal(_fence, ++_fenceVal);
		if (_fence->GetCompletedValue() != _fenceVal) 
		{
			// イベントハンドルの取得
			auto event = CreateEvent(nullptr, false, false, nullptr);

			_fence->SetEventOnCompletion(_fenceVal, event);

			// イベントが発生するまで待ち続ける
			WaitForSingleObject(event, INFINITE);

			// イベントハンドルを閉じる
			CloseHandle(event);
		}


		// 実行が終わればコマンドリストは不要となるので中身をクリアする
		_cmdAllocator->Reset();	// キューをクリア
		_cmdList->Reset(_cmdAllocator, nullptr);	// 再びコマンドリストをためる準備

		// 直前でレンダーターゲットからPresentに移行する必要がある
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		// 画面のスワップ
		_swapchain->Present(1, 0);	// 第一引数は、フリップまでの待ちフレーム数



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

#include <Windows.h>
#include <tchar.h>
#include <vector>

// DirectX��DSGI�𐧌䂷�邽�߂ɕK�v�ȃw�b�_�[�̃C���N���[�h
#include <d3d12.h>
#include <dxgi1_6.h>

// ���C�u�����̃����N
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#ifdef _DEBUG
#include <iostream>
#endif 

using namespace std;

// @brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @param format�t�H�[�}�b�g�i%d�Ƃ���%f�Ƃ��́j
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�O�p�ł��B�f�o�b�O���ɂ������삵�܂���
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

// �ʓ|�����Ǐ����Ȃ���΂����Ȃ��֐�
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);	// OS�ɑ΂��āu�������̃A�v���͏I���Ɠ`����v
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);	// ����̏������s��
}

// �f�o�b�O���C���[��L��������֐�
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));

	debugLayer->EnableDebugLayer();	// �f�o�b�O���C���[��L��������
	debugLayer->Release();	// �L����������C���^�[�t�F�C�X���J������
}

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif

	// �C�ӂ̃E�B���h�E�T�C�Y��ݒ�
	int window_width = 1600;
	int window_height = 900;

	// �E�B���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};

	// �E�B���h�E�쐬
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	// �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample");			// �A�v���P�[�V�����N���X���i�K���ł悢�j
	w.hInstance = GetModuleHandle(nullptr);		// �n���h���̎擾

	RegisterClassEx(&w);	// �A�v���P�[�V�����N���X(�E�B���h�E�N���X�̎w���OS�ɓ`����)

	RECT wrc = { 0,0,window_width,window_height };	// �E�B���h�E�T�C�Y�����߂�

	// �֐����g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,	// �N���X���w��
		_T("DX12�e�X�g"),			// �^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,	// �^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,			// �\��x���W��OS�ɂ��C��
		CW_USEDEFAULT,			// �\��y���W��OS�ɂ��C��
		wrc.right - wrc.left,	// �E�B���h�E��
		wrc.bottom - wrc.top,	// �E�B���h�E��
		nullptr,				// �e�E�B���h�E�n���h��
		nullptr,				// ���j���[�n���h��
		w.hInstance,			// �Ăяo���A�v���P�[�V�����n���h��
		nullptr);

	// �E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	// �f�o�b�O���C���[��L���ɂ���
#ifdef _DEBUG
	// �f�o�b�O���C���[���I��
	EnableDebugLayer();
#endif 

	// DirectX�̏�����


	/*DirectX��{�����̏�����*/

	// �󂯎M�ƂȂ�ϐ��̗p��
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	// �R�}���h���X�g�ƃR�}���h�A���P�[�^�[�̐錾
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;

	// �O���t�B�b�N�X�{�[�h�������h�����Ă���ꍇ�̏���

	// DXGIFactory�I�u�W�F�N�g�𐶐�����
#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));	// DXGIFactory��_dxgiFactory�ɓ���
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));	// DXGIFactory��_dxgiFactory�ɓ���
#endif

	// �A�_�v�^�[�̗񋓗p
	std::vector <IDXGIAdapter*> adapters;

	// �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tempAdapter = nullptr;

	// �A�_�v�^�[��z��ɕۑ����Ă���
	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tempAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(tempAdapter);
	}

	// ����̃A�_�v�^�[��T���o��
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);	// �A�_�v�^�[�̐����I�u�W�F�N�g���擾

		std::wstring strDesc = adesc.Description;	// �A�_�v�^�[�̖��O���擾

		// �T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos)	// ���̃R�[�h��NVIDIA���g���Ă��邩��i�z�z�⑼��PC���Ƃ��܂�悭�Ȃ��j
		{
			tempAdapter = adpt;
			break;
		}
	}

	// �t�B�[�`���[���x���\����
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	// Direct3D�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featurelevel;

	// �g�p�ł���o�[�W�����������ăf�o�C�X�I�u�W�F�N�g�𐶐�����
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tempAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featurelevel = lv;
			break;	// �����\�ȃo�[�W���������������烋�[�v��ł��؂�
		}
	}

	// ���X�g�ƃA���P�[�^�[�̐���
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));

	// �R�}���h�L���[�̐錾
	ID3D12CommandQueue* _cmdQueue = nullptr;

	// �L���[�̐ݒ�̂��߂̍\����
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	// �^�C���A�E�g�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// �A�_�v�^�[��������g��Ȃ��Ƃ��͂O�ł���	
	cmdQueueDesc.NodeMask = 0;

	// �v���C�I���e�B�͓��ɂȂ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	// �R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// �L���[�̐���
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	// �X���b�v�`�F�[���̍\����
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	// �K�p����ݒ�
	swapchainDesc.Width = window_width;					// ��ʉ𑜓x(��)
	swapchainDesc.Height = window_height;				// ��ʉ𑜓x(��)
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// �s�N�Z���t�H�[�}�b�g
	swapchainDesc.Stereo = false;						// �X�e���I�\���v���O(3D�f�B�X�v���C�̃X�e���I���[�h)
	swapchainDesc.SampleDesc.Count = 1;					// �}���`�T���v���̎w��
	swapchainDesc.SampleDesc.Quality = 0;				// �}���`�T���v���̎w��
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;	// DXGI_USAGE_BACK_BUFFER�ł悢
	swapchainDesc.BufferCount = 2;						// �_�u���o�b�t�@�[�Ȃ�Q�ł悢

	// �o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	// �t���b�v��͑��₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Alpha���[�h�̎w��͓��ɂȂ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// �E�B���h�E�̃t���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// �X���b�v�`�F�[���̐���
	result = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&_swapchain);

	// �f�B�X�N�v���^�q�[�v�̍\����
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// �����_�[�^�[�Q�b�g�r���[�Ȃ̂�RTV
	heapDesc.NodeMask = 0;	// ������GPU���g�p����ꍇ�A���ʂ��邽�߂̂���
	heapDesc.NumDescriptors = 2;	//�\���̓��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	// ���Ɏw��Ȃ�

	// �f�B�X�N�v���^�q�[�v�̐���
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	// �X���b�v�`�F�[���̃������ƕR�t����
	
	// �\���̂̐錾
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	
	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	for(int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		// �X���b�v�`�F�[����̃o�b�N�o�b�t�@�[��buckBuffer���擾
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));

		// �f�B�X�N�v���^���K�v�ȃT�C�Y�͂��ꂼ��Ⴄ���ߒ��꒲�ׂăT�C�Y�����炷
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	}

	// �R�}���h�A���P�[�^�[�̖��߃I�u�W�F�N�g���N���A����
	result = _cmdAllocator->Reset();

	MSG msg = {};
	// �A�v���P�[�V�������I�����Ȃ����߂̖������[�v
	while (true)
	{
		// �o�b�N�o�b�t�@�[���w���C���f�b�N�X���擾
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		// ��̃C���f�b�N�X�Ŏ����ꂽ�����_�[�^�[�Q�b�g�r���[�����ꂩ��g�����̂Ƃ��ăZ�b�g����
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		
		// �o���A�̍\���̂��쐬
		D3D12_RESOURCE_BARRIER BarrierDesc = {};

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	// �J��
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	// ���Ɏw��Ȃ�
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];		// �o�b�N�o�b�t�@�[���\�[�X
		BarrierDesc.Transition.Subresource = 0;

		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;	// ���O��PRESENT���
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	// �����烌���_�[�^�[�Q�b�g���

		_cmdList->ResourceBarrier(1, &BarrierDesc);	// �o���A�w����s

		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		// �����_�[�^�[�Q�b�g�����̐F�ŃN���A����

		// ��ʂ��N���A����F���w��
		float clearColor[] = { 0.0f,0.0f,1.0f,1.0f };	// ���F
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);	// ���A��O�����̓N���A����͈�

		// ���߂Ă������߂����s����

		// ���߂̃N���[�Y(�K�{)
		_cmdList->Close();

		// �N���[�Y������ɃR�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { _cmdList };

		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		// �t�F���X�̍쐬
		ID3D12Fence* _fence = nullptr;
		UINT64 _fenceVal = 0;

		result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

		// GPU�̏������I���܂�
		_cmdQueue->Signal(_fence, ++_fenceVal);
		if (_fence->GetCompletedValue() != _fenceVal) 
		{
			// �C�x���g�n���h���̎擾
			auto event = CreateEvent(nullptr, false, false, nullptr);

			_fence->SetEventOnCompletion(_fenceVal, event);

			// �C�x���g����������܂ő҂�������
			WaitForSingleObject(event, INFINITE);

			// �C�x���g�n���h�������
			CloseHandle(event);
		}


		// ���s���I���΃R�}���h���X�g�͕s�v�ƂȂ�̂Œ��g���N���A����
		_cmdAllocator->Reset();	// �L���[���N���A
		_cmdList->Reset(_cmdAllocator, nullptr);	// �ĂуR�}���h���X�g�����߂鏀��

		// ���O�Ń����_�[�^�[�Q�b�g����Present�Ɉڍs����K�v������
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		// ��ʂ̃X���b�v
		_swapchain->Present(1, 0);	// �������́A�t���b�v�܂ł̑҂��t���[����



		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// �A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	











	// �����N���X�͎g��Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);

	//DebugOutputFormatString("Show window test.");
	//getchar();
	return 0;
}

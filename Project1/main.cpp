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
	printf(format, valist);
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

// �E�B���h�E�쐬
void MakeWindow(WNDCLASSEX& w)
{
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	// �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample");			// �A�v���P�[�V�����N���X���i�K���ł悢�j
	w.hInstance = GetModuleHandle(nullptr);		// �n���h���̎擾

	RegisterClassEx(&w);	// �A�v���P�[�V�����N���X(�E�B���h�E�N���X�̎w���OS�ɓ`����)

	// �C�ӂ̃E�B���h�E�T�C�Y��ݒ�
	int window_width = 1600;
	int window_height = 900;

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
}

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif

	// �E�B���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};

	// �E�B���h�E�쐬
	MakeWindow(w);

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
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));	// DXGIFactory��_dxgiFactory�ɓ���

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
			featurelevel == lv;
			break;	// �����\�ȃo�[�W���������������烋�[�v��ł��؂�
		}
	}

	


	MSG msg = {};
	// �A�v���P�[�V�������I�����Ȃ����߂̖������[�v
	while (true)
	{
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

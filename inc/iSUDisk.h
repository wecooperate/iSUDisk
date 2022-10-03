//******************************************************************************
/*++
	FileName:		iSUDisk.h
	Description:

--*/
//******************************************************************************
#ifndef __iSUDisk_h_af9dd655_57b5_45e9_bbbf_170d74a17f30__
#define __iSUDisk_h_af9dd655_57b5_45e9_bbbf_170d74a17f30__
//******************************************************************************
// clang-format off
//******************************************************************************
#include <tchar.h>
#include <windows.h>
#include <atlbase.h>
//******************************************************************************
#define SUDISK_MODULE_NAME _T("iSUDisk.dll")
//******************************************************************************
#define SUDISK_IID_MOUNT_CONTEXT "{53FDD3A8-EED6-406E-805C-8006897FB340}"
#define SUDISK_IID_SUDISK "{53FDD3A8-EED6-406E-805C-8006897FB341}"
//******************************************************************************
#ifndef BEGIN_NAMESPACE_SUDISK
#define BEGIN_NAMESPACE_SUDISK
#define END_NAMESPACE_SUDISK
#endif
//******************************************************************************
BEGIN_NAMESPACE_SUDISK
//******************************************************************************
enum {
	emSUDiskMaxString = 64,
	emSUDiskMaxBuffer = 1024,
};
//******************************************************************************
enum emSUDiskEncryptType
{
	emSUDiskEncryptNone,
	emSUDiskEncryptQuick,
	emSUDiskEncryptAES128,
	emSUDiskEncryptAES256,
	emSUDiskEncryptSM4,
};
//******************************************************************************
struct SUDiskRegistration 
{
	emSUDiskEncryptType EncryptType;
	ULONG Reserved;
	ULONGLONG DiskSize;
	LPCWSTR Password;
	LPCWSTR DiskLabel;			// MAX_LENGTH = emSUDiskMaxString
	LPCWSTR DiskRegisterName;	// MAX_LENGTH = emSUDiskMaxString
	UCHAR DiskCustomData[emSUDiskMaxBuffer];
};
//******************************************************************************
struct SUDiskInfo 
{
	emSUDiskEncryptType EncryptType;
	ULONG IsNeedPassword : 1;
	ULONG Reserved : 31;
	WCHAR DiskLabel[emSUDiskMaxString];
	WCHAR DiskRegisterName[emSUDiskMaxString];
	UCHAR DiskCustomData[emSUDiskMaxBuffer];
};
//******************************************************************************
interface __declspec (uuid(SUDISK_IID_MOUNT_CONTEXT)) ISUDiskMountContext : public IUnknown
{
	virtual LPCWSTR	GetMountPath		(void) = 0;
	virtual HRESULT	SetAutoUnmount		(bool Enable) = 0;
	virtual HRESULT	ExploreMountPath	(void) = 0;
};
//******************************************************************************
interface __declspec (uuid(SUDISK_IID_SUDISK)) ISUDisk : public IUnknown
{
	//
	// DiskPath:
	//		X: 路径为根目录，表示U盘
	//		X:\\disk.isu 路径为文件路径，表示文件磁盘
	//
	// MountPath:
	//		X: 路径为根目录，表示挂载成磁盘
	//		X:\\disk 路径为目录，表示挂载到目录（目录必须为空目录）
	//		NULL 空路径，表示自动挂载到空闲的磁盘，挂载后的路径可以从ISUDiskMountContext.GetMountPath获取
	//

	virtual HRESULT Register			(LPCWSTR DiskPath, const SUDiskRegistration& Registration) = 0;
	virtual HRESULT Unregister			(LPCWSTR DiskPath) = 0;
	virtual HRESULT	Mount				(LPCWSTR DiskPath, LPCWSTR MountPath, LPCWSTR Password, ISUDiskMountContext** Context) = 0;
	virtual HRESULT MountRamDisk		(ULONGLONG DiskSize, LPCWSTR MountPath, ISUDiskMountContext** Context) = 0;
	virtual HRESULT Unmount				(LPCWSTR MountPath) = 0;
	virtual HRESULT GetDiskInfo			(LPCWSTR DiskPath, SUDiskInfo* Info) = 0;
	virtual HRESULT GetVerifiedDiskInfo	(LPCWSTR DiskPath, LPCWSTR Password, SUDiskInfo* Info) = 0;
	virtual HRESULT	ResetPassword		(LPCWSTR DiskPath, LPCWSTR Password, LPCWSTR NewPassword) = 0;
};
//******************************************************************************
// clang-format on
//******************************************************************************
class SUDisk
{
public:
	SUDisk(void)
		: m_Module(NULL)
	{
	}

	~SUDisk(void)
	{
		Uninitialize();

		m_Instance = NULL;
	}

	HRESULT Initialize(LPCTSTR Path = SUDISK_MODULE_NAME)
	{
		HRESULT hr = LoadModule(Path);

		if (hr != S_OK)
			return hr;

		if (!m_Instance)
			return E_UNEXPECTED;

		return S_OK;
	}

	HRESULT Uninitialize(void)
	{
		if (m_Instance) {
			m_Instance.Release();
		}

		return S_OK;
	}

public:
	ISUDisk* operator->(void)
	{
		return m_Instance;
	}

protected:
	HRESULT LoadModule(LPCTSTR Path)
	{
		if (m_Instance)
			return S_OK;

		if (!Path) {
			Path = SUDISK_MODULE_NAME;
		}

		if (!m_Module) {
			m_Module = LoadLibraryEx(Path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

			if (!m_Module)
				return HRESULT_FROM_WIN32(GetLastError());
		}

		typedef HRESULT(STDAPICALLTYPE * PFN_DllGetClassObject)(REFCLSID, REFIID, PVOID*);

		PFN_DllGetClassObject pfn = (PFN_DllGetClassObject)GetProcAddress(m_Module, "DllGetClassObject");

		if (!pfn)
			return E_FAIL;

		return pfn(CLSID_NULL, __uuidof(ISUDisk), (PVOID*)&m_Instance);
	}

protected:
	HMODULE m_Module;
	CComPtr<ISUDisk> m_Instance;
};
//******************************************************************************
END_NAMESPACE_SUDISK
//******************************************************************************
#endif

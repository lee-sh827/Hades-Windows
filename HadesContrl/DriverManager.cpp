#include "DriverManager.h"
#include <Windows.h>
#include <xstring>

DriverManager::DriverManager()
{

}

DriverManager::~DriverManager()
{

}

#define				SECURITY_STRING_LEN							168
#define				LG_PAGE_SIZE								4096
#define				MAX_KEY_LENGTH								1024
#define				LG_SLEEP_TIME								4000

const BYTE g_szSecurity[SECURITY_STRING_LEN] =
{
	0x01,0x00,0x14,0x80,0x90,0x00,0x00,0x00,0x9c,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x02,
	0x00,0x1c,0x00,0x01,0x00,0x00,0x00,0x02,0x80,0x14,0x00,0xff,0x01,0x0f,0x00,0x01,0x01,0x00,0x00,0x00,0x00,
	0x00,0x01,0x00,0x00,0x00,0x00,0x02,0x00,0x60,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0xfd,0x01,0x02,
	0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x12,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0xff,0x01,0x0f,0x00,
	0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x05,0x20,0x00,0x00,0x00,0x20,0x02,0x00,0x00,0x00,0x00,0x14,0x00,0x8d,
	0x01,0x02,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x0b,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0xfd,0x01,
	0x02,0x00,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x05,0x20,0x00,0x00,0x00,0x23,0x02,0x00,0x00,0x01,0x01,0x00,
	0x00,0x00,0x00,0x00,0x05,0x12,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x12,0x00,0x00,0x00
};

int	InstallDriver(const wchar_t* cszDriverName, const wchar_t* cszDriverFullPath)
{
	WCHAR	szBuf[LG_PAGE_SIZE];
	HKEY	hKey;
	DWORD	dwData;

	if (NULL == cszDriverName || NULL == cszDriverFullPath)
	{
		return -1;
	}
	memset(szBuf, 0, LG_PAGE_SIZE);
	lstrcpyW(szBuf, L"SYSTEM\\CurrentControlSet\\Services\\");
	lstrcatW(szBuf, cszDriverName);
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szBuf, 0, REG_NONE, 0, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		return -1;
	}
	lstrcpyW(szBuf, cszDriverName);
	if (RegSetValueEx(hKey, L"DisplayName", 0, REG_SZ, (CONST BYTE*)szBuf, (DWORD)lstrlenW(szBuf)) != ERROR_SUCCESS)
	{
		return -1;
	}
	dwData = 1;
	if (RegSetValueEx(hKey, L"ErrorControl", 0, REG_DWORD, (CONST BYTE*) & dwData, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		return -1;
	}
	lstrcpyW(szBuf, L"\\??\\");
	lstrcatW(szBuf, cszDriverFullPath);
	if (RegSetValueEx(hKey, L"ImagePath", 0, REG_SZ, (CONST BYTE*)szBuf, (DWORD)lstrlenW(szBuf)) != ERROR_SUCCESS)
	{
		return -1;
	}
	dwData = 3;
	if (RegSetValueEx(hKey, L"Start", 0, REG_DWORD, (CONST BYTE*) & dwData, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		return -1;
	}
	dwData = 1;
	if (RegSetValueEx(hKey, L"Type", 0, REG_DWORD, (CONST BYTE*) & dwData, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		return -1;
	}
	RegFlushKey(hKey);
	RegCloseKey(hKey);
	lstrcpyW(szBuf, L"SYSTEM\\CurrentControlSet\\Services\\");
	lstrcpyW(szBuf, cszDriverName);
	lstrcpyW(szBuf, L"\\Security");
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szBuf, 0, REG_NONE, 0, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		return -1;
	}
	dwData = SECURITY_STRING_LEN;
	if (RegSetValueEx(hKey, L"Security", 0, REG_BINARY, g_szSecurity, dwData) != ERROR_SUCCESS)
	{
		return -1;
	}
	RegFlushKey(hKey);
	RegCloseKey(hKey);

	return 0;
}

int CreateDriver(const wchar_t* cszDriverName, const wchar_t* cszDriverFullPath)
{
	SC_HANDLE		schManager;
	SC_HANDLE		schService;
	SERVICE_STATUS	svcStatus;
	bool			bStopped = false;
	int				i;

	if (NULL == cszDriverName || NULL == cszDriverFullPath)
	{
		return -1;
	}
	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		return -1;
	}
	schService = OpenService(schManager, cszDriverName, SERVICE_ALL_ACCESS);
	if (NULL != schService)
	{
		if (ControlService(schService, SERVICE_CONTROL_INTERROGATE, &svcStatus))
		{
			if (svcStatus.dwCurrentState != SERVICE_STOPPED)
			{
				if (0 == ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus))
				{
					CloseServiceHandle(schService);
					CloseServiceHandle(schManager);
					return -1;
				}
				for (i = 0; i < 10; i++)
				{
					if (ControlService(schService, SERVICE_CONTROL_INTERROGATE, &svcStatus) == 0 || svcStatus.dwCurrentState == SERVICE_STOPPED)
					{
						bStopped = true;
						break;
					}
					Sleep(LG_SLEEP_TIME);
				}
				if (!bStopped)
				{
					CloseServiceHandle(schService);
					CloseServiceHandle(schManager);
					return -1;
				}
			}
		}
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return 0;
	}
	schService = CreateService(schManager, cszDriverName, cszDriverName, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, cszDriverFullPath, NULL, NULL, NULL, NULL, NULL);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return -1;
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return 0;
}

int StartDriver(const wchar_t* cszDriverName, const wchar_t* cszDriverFullPath)
{
	SC_HANDLE		schManager;
	SC_HANDLE		schService;
	SERVICE_STATUS	svcStatus;
	bool			bStarted = false;
	int				i;

	if (NULL == cszDriverName)
	{
		return -1;
	}
	if (CreateDriver(cszDriverName, cszDriverFullPath) < 0)
	{
		return -1;
	}
	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		return -1;
	}
	schService = OpenService(schManager, cszDriverName, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return -1;
	}
	if (ControlService(schService, SERVICE_CONTROL_INTERROGATE, &svcStatus))
	{
		if (svcStatus.dwCurrentState == SERVICE_RUNNING)
		{
			CloseServiceHandle(schService);
			CloseServiceHandle(schManager);
			return 0;
		}
	}
	else if (GetLastError() != ERROR_SERVICE_NOT_ACTIVE)
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return -1;
	}
	if (0 == StartService(schService, 0, NULL))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return -1;
	}
	for (i = 0; i < 10; i++)
	{
		if (ControlService(schService, SERVICE_CONTROL_INTERROGATE, &svcStatus) && svcStatus.dwCurrentState == SERVICE_RUNNING)
		{
			bStarted = true;
			break;
		}
		Sleep(LG_SLEEP_TIME);
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return bStarted ? 1 : -1;
}

int StopDriver(const wchar_t* cszDriverName, const wchar_t* cszDriverFullPath)
{
	SC_HANDLE		schManager;
	SC_HANDLE		schService;
	SERVICE_STATUS	svcStatus;
	bool			bStopped = false;
	int				i;

	schManager = OpenSCManager(NULL, 0, 0);
	if (NULL == schManager)
	{
		return -1;
	}
	schService = OpenService(schManager, cszDriverName, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return -1;
	}
	if (ControlService(schService, SERVICE_CONTROL_INTERROGATE, &svcStatus))
	{
		if (svcStatus.dwCurrentState != SERVICE_STOPPED)
		{
			if (0 == ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus))
			{
				CloseServiceHandle(schService);
				CloseServiceHandle(schManager);
				return -1;
			}
			for (i = 0; i < 10; i++)
			{
				if (ControlService(schService, SERVICE_CONTROL_INTERROGATE, &svcStatus) == 0 || svcStatus.dwCurrentState == SERVICE_STOPPED)
				{
					bStopped = true;
					break;
				}
				Sleep(LG_SLEEP_TIME);
			}
			if (!bStopped)
			{
				CloseServiceHandle(schService);
				CloseServiceHandle(schManager);
				return -1;
			}
		}
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return 0;
}

int GetServicesStatus(const wchar_t* driverName)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint = 0;
	DWORD dwStartTickCount = 0;
	DWORD dwWaitTime = 0;
	DWORD dwBytesNeeded = 0;

	schSCManager = OpenSCManager(
		NULL,                                // local computer
		NULL,                                // ServicesActive database
		SC_MANAGER_ALL_ACCESS);              // full access rights

	if (NULL == schSCManager)
	{
		OutputDebugStringA("hades OpenSCManager error");
		return -1;

	}

	schService = OpenService(
		schSCManager,                      // SCM database
		driverName,                         // name of service
		SERVICE_QUERY_STATUS |
		SERVICE_ENUMERATE_DEPENDENTS);     // full access

	if (schService == NULL)
	{
		OutputDebugStringA("OpenService failed");
		CloseServiceHandle(schSCManager);
		return -1;
	}

	if (!QueryServiceStatusEx(
		schService,                         // handle to service
		SC_STATUS_PROCESS_INFO,             // information level
		(LPBYTE)&ssStatus,                 // address of structure
		sizeof(SERVICE_STATUS_PROCESS),     // size of structure
		&dwBytesNeeded))                  // size needed if buffer is too small
	{
		OutputDebugStringA("QueryServiceStatusEx failed");
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return -1;
	}
	return ssStatus.dwCurrentState;
}

int DeleteDriver(const wchar_t* cszDriverName, const wchar_t* cszDriverFullPath)
{
	bool nSeriverstatus = GetServicesStatus(cszDriverName);
	switch (nSeriverstatus)
	{
	// 正在运行
	case SERVICE_CONTINUE_PENDING:
	case SERVICE_RUNNING:
	case SERVICE_START_PENDING:
	{
		StopDriver(cszDriverName, cszDriverFullPath);
		Sleep(200);
		break;
	}
	break;
	}

	// 卸载
	SC_HANDLE    schManager;
	SC_HANDLE    schService;
	SERVICE_STATUS  svcStatus;

	schManager = OpenSCManager(NULL, 0, 0);
	if (NULL == schManager)
	{
		return -1;
	}
	schService = OpenService(schManager, cszDriverName, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return -1;
	}
	ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus);
	if (0 == DeleteService(schService))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return -1;
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	// 删除文件
	DeleteFile(cszDriverFullPath);
	return 0;
}

LONG RegDeleteKeyNT(HKEY hStartKey, LPTSTR pKeyName)
{
	DWORD  dwSubKeyLength;
	LPTSTR  pSubKey = NULL;
	TCHAR  szSubKey[MAX_KEY_LENGTH];
	HKEY  hKey;
	LONG  lRet;

	if (pKeyName && lstrlen(pKeyName))
	{
		if ((lRet = RegOpenKeyEx(hStartKey, pKeyName, 0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey)) == ERROR_SUCCESS)
		{
			while (lRet == ERROR_SUCCESS)
			{
				dwSubKeyLength = MAX_KEY_LENGTH;
				lRet = RegEnumKeyEx(hKey, 0, szSubKey, (LPDWORD)&dwSubKeyLength, NULL, NULL, NULL, NULL);
				if (lRet == ERROR_NO_MORE_ITEMS)
				{
					lRet = RegDeleteKey(hStartKey, pKeyName);
					break;
				}
				else if (lRet == ERROR_SUCCESS)
				{
					lRet = RegDeleteKeyNT(hKey, szSubKey);
				}
			}
			RegCloseKey(hKey);
		}
	}
	else
	{
		lRet = ERROR_BADKEY;
	}

	return lRet;
}

int RemoveDriver(const wchar_t* cszDriverName, const wchar_t* cszDriverFullPath)
{
	HKEY hKey;
	long errorno;
	char szBuf[LG_PAGE_SIZE];
	wchar_t szDriverName[MAX_PATH];

	memset(szBuf, 0, LG_PAGE_SIZE);
	memset(szDriverName, 0, MAX_PATH);
	lstrcpyW(szDriverName, cszDriverName);
	strcpy(szBuf, "SYSTEM//CurrentControlSet//Services//");
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, szBuf, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		return -1;
	}
	if ((errorno = RegDeleteKeyNT(hKey, szDriverName)) != ERROR_SUCCESS)
	{
		return -1;
	}
	RegCloseKey(hKey);
	return 0;
}

int DriverManager::nf_GetServicesStatus(const wchar_t* driverName)
{
	return GetServicesStatus(driverName);
}

int DriverManager::nf_StartDrv(const wchar_t* cszDriverName = L"hadesmondrv", const wchar_t* cszDriverFullPath = L"C:\\Windows\\System32\\drivers\\sysmondriver.sys")
{
	return StartDriver(cszDriverName, cszDriverFullPath);
}

int DriverManager::nf_StopDrv(const wchar_t* cszDriverName = L"hadesmondrv", const wchar_t* cszDriverFullPath = L"C:\\Windows\\System32\\drivers\\sysmondriver.sys")
{
	return StopDriver(cszDriverName, cszDriverFullPath);
}

int DriverManager::nf_DeleteDrv(const wchar_t* cszDriverName = L"hadesmondrv", const wchar_t* cszDriverFullPath = L"C:\\Windows\\System32\\drivers\\sysmondriver.sys")
{
	return DeleteDriver(cszDriverName, cszDriverFullPath);
}

bool DriverManager::nf_DriverInstall(const int mav, const int miv, const bool Is64)
{
	// win7以下不支持
	if (mav < 6)
		return false;
	std::wstring DriverPath;
	std::wstring PathAll;
	std::wstring pszCmd = L"sc start hadesmondrv";
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	DWORD nSeriverstatus = -1;
	TCHAR szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	OutputDebugString(szFilePath);
	DriverPath = szFilePath;
	int num = DriverPath.find_last_of(L"\\");
	PathAll = DriverPath.substr(0, num);

	// 如果存在会返回ERROR_ALREADY_EXISTS
	std::wstring direct;
	direct = L"C:\\Windows\\sysnative";
	CreateDirectory(direct.c_str(), NULL);
	direct = L"C:\\Windows\\sysnative\\drivers";
	CreateDirectory(direct.c_str(), NULL);

	switch (miv)
	{
	case 0://win7
	{
		if(Is64)
			PathAll += L"driver\\win7\\64\\sysmondriver.sys";
		else
			PathAll += L"driver\\sysmondriver.sys";
	}
	break;
	case 1://win8
	case 2://win10
	case 3://win11
	{
		if (Is64)
			PathAll += L"driver\\win10\\64\\sysmondriver.sys";
		else
			PathAll += L"driver\\win10\\sysmondriver.sys";
	}
	break;
	default:
		return false;
	}
	
	// 先拷贝到C盘 - 无论32/64统一驱动名sysmondriver
	CopyFile(PathAll.data(), L"C:\\Windows\\sysnative\\drivers\\sysmondriver.sys", FALSE);

	//if (InstallDriver(L"hadesmondrv", L"C:\\Windows\\System32\\drivers\\wfpdriver.sys") == TRUE) {
	//	OutputDebugString(L"installDvr success.");
	//}
	//else
	//{
	//	return -1;
	//}
	if (StartDriver(L"hadesmondrv", L"C:\\Windows\\sysnative\\drivers\\sysmondriver.sys") == TRUE)
	{
		OutputDebugString(L"Start Driver success.");
		return true;
	}
	else
	{
		OutputDebugString(L"Start Driver failuer.");
		return false;
	}
}


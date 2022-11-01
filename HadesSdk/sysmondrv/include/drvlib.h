#ifndef _DEVCTRL_H
#define _DEVCTRL_H

#include <queue>
#include <mutex>
#include "sysinfo.h"

// 采集开启
#define CTL_DEVCTRL_ENABLE_MONITOR \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
// 共享内存
#define CTL_DEVCTRL_OPEN_SHAREMEM \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
// 采集关闭
#define CTL_DEVCTRL_DISENTABLE_MONITOR \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
// IPS Process Name  
#define CTL_DEVCTRL_IPS_SETPROCESSNAME \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
// IPS Process Mods
#define CTL_DEVCTRL_IPS_SETPROCESSFILTERMOD \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)
// 规则启用开关
#define CTL_DEVCTRL_ENABLE_IPS_MONITOR \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CTL_DEVCTRL_DISENTABLE_IPS_MONITOR \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
// IPS Register Name  
#define CTL_DEVCTRL_IPS_SETREGISTERNAME \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)
// IPS Register Mods
#define CTL_DEVCTRL_IPS_SETREGISTERFILTERMOD \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
// IPS Driectory Mods/Name
#define CTL_DEVCTRL_IPS_SETDIRECTORYRULE \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)
// IPS ThreadInject Name  
#define CTL_DEVCTRL_IPS_SETTHREADINJECTNAME \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef enum _NF_DRIVER_TYPE
{
	DT_UNKNOWN = 0,
	DT_TDI = 1,
	DT_WFP = 2
} NF_DRIVER_TYPE;

/**
*	Internal IO structure
**/
typedef struct _NF_DATA
{
	int				code;
	int				id;
	unsigned long	bufferSize;
	char 			buffer[1];
} NF_DATA, * PNF_DATA;

typedef  struct _NF_BUFFERS
{
	unsigned __int64 inBuf;
	unsigned __int64 inBufLen;
	unsigned __int64 outBuf;
	unsigned __int64 outBufLen;
} NF_BUFFERS, * PNF_BUFFERS;

typedef  struct _NF_READ_RESULT
{
	unsigned __int64 length;
} NF_READ_RESULT, * PNF_READ_RESULT;

class DevctrlIoct
{
public:
	DevctrlIoct();
	~DevctrlIoct();

	int devctrl_init();
	void devctrl_free();
	void devctrl_stopthread();
	void devctrl_startthread();
	int devctrl_opendeviceSylink(const char* devSylinkName);
	int devctrl_workthread(LPVOID grpcobj, const bool flag);
	int devctrl_waitSingeObject();
	int devctrl_InitshareMem();
	void nf_setEventHandler(PVOID64 pHandler);
	int devctrl_OnMonitor();
	int devctrl_OffMonitor();
	int devctrl_OnIpsMonitor();
	int devctrl_OffIpsMonitor();
	int devctrl_SetIpsProcessNameList(const DWORD code, const wchar_t* buf);
	int devctrl_SetIpsFilterMods(const DWORD32 code, const int mods);

	bool devctrl_sendioct(
		const int ioctcode,
		LPVOID lpInBuffer,
		DWORD InBufSize,
		LPVOID lpOutBuffer,
		DWORD OutBufSize,
		DWORD& dSize
	);

	void kf_setqueuetaskptr(std::queue<UPubNode*>& qptr);
	void kf_setqueuelockptr(std::mutex& qptrcs);
	void kf_setqueueeventptr(HANDLE& eventptr);

private:
	HANDLE m_devhandler;
	HANDLE m_threadobjhandler;
	HANDLE m_alpcthreadobjhandler;
	HANDLE m_listthreadobjhandler;
	DWORD  m_dwthreadid;
	DWORD  m_dwthreadid1;
	int devctrl_writeio();
	PVOID get_eventhandler();

};

#endif // !_DEVCTRL_H
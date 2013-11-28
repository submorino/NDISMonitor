
#include "precomp.h"
#pragma hdrstop

extern LONG		KeReadStateEvent(IN PRKEVENT Event);
extern PVOID	gpEventObject;			// 与应用通信的事件对象
extern PVOID	SystemVirtualAddress;	// 共享内存地址
extern UCHAR	filt[13];

unsigned short ntohs(unsigned short netshort)
{
	return (((netshort<<8)&0xff00)+((netshort>>8)&0x00ff));
}

unsigned long ntohl(unsigned long netlong)
{
	return(((netlong>>24)&0x000000ff)+((netlong<<24)&0xff000000)+((netlong>>8)&0x0000ff00)+((netlong<<8)&0x00ff0000));
}

UINT Filter(UCHAR *pPacket)
{
	UCHAR *ppkt = pPacket;
	UCHAR *pfilt = filt;
	if(*(ppkt+12) == 0x08 && *(ppkt+13) == 0x00)
	{
		if((*pfilt == 0) || (*pfilt == *(ppkt+23)))
			if((*(ULONG*)(pfilt+1) == 0) || (*(ULONG*)(pfilt+1) == ntohl(*(ULONG*)(ppkt+26))))
				if((*(ULONG*)(pfilt+7) == 0) || (*(ULONG*)(pfilt+7) == ntohl(*(ULONG*)(ppkt+30))))
				{
					if((*pfilt == 6)||(*pfilt == 17))
					{
						if((*(USHORT*)(pfilt+5) == 0) || (*(USHORT*)(pfilt+5) == *(USHORT*)(ppkt+34)))
							if((*(USHORT*)(pfilt+11) == 0) || (*(USHORT*)(pfilt+11) == *(USHORT*)(ppkt+34)))
								return 1;
							return 0;
					}
					return 1;
				}
	}
	return 0;
}

VOID WritePacket2SharedMemory(UCHAR *pPacket,UINT packetsize)
{
	if(!KeReadStateEvent(gpEventObject))
				{
		// 复制数据到共享内存
		memset(SystemVirtualAddress,0, MAX_PACKET_SIZE+sizeof(USHORT));
		memcpy((USHORT*)SystemVirtualAddress, &packetsize, sizeof(USHORT));
		memcpy((UCHAR*)SystemVirtualAddress+sizeof(USHORT), pPacket, packetsize);
		
		KeSetEvent(gpEventObject, 0, FALSE);
				}
	return;
}


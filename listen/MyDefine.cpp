#include "stdafx.h"
#include "MyDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString GetNowTime()
{
	CTime time;
	CString str;
	time = CTime::GetCurrentTime();	
	str = time.Format("%Y-%m-%d %H:%M:%S");
	return str;
}

CString QueryProtocol(BYTE iProtocol)
{
	iProtocol = iProtocol&0xff;
	for(int i=0; i<MAX_PROTO_NUM; i++)
	{
		if(ProtoMap[i].ProtoNum==iProtocol)
		{
			return ProtoMap[i].ProtoText;
		}
	}
		return "";
}

BYTE QueryProtocolNum(CString csProtocol)
{
	for(int i=0; i<MAX_PROTO_NUM; i++)
	{
		if(0 == strcmp(ProtoMap[i].ProtoText,csProtocol))
			return ProtoMap[i].ProtoNum;
	}
	return 0;
}

CString BytestoString(BYTE *pbdata,int count)
{
	BYTE *pbdata1 = pbdata;
	int byteData;
	int LOByte,HIByte;
	char stra[100];
	CString cstr;
	char *str=stra;
	for(int j=0;j<count;j++)
	{	
		byteData = (int)(*pbdata1);
		LOByte=(byteData & 0xf);	
		HIByte=(byteData>>4) & 0xf;
		itoa(HIByte,str++,16);
		itoa(LOByte,str++,16);
		*(str++) = '-';

		pbdata1++;
	}
	stra[j]='\0';
	cstr.Format("%s",stra);
	
	return cstr;
}

CString GetIPFromBytes(BYTE *pbyteData, int count)
{
	BYTE *pbdata1 = pbyteData;
	int byteData;
//	char stra[100];
	CString cstr;
	CString tstr;

	for(int j=0;j<count-1;j++)
	{	
		byteData = (int)(*pbdata1) & 0xff;
		tstr.Format("%d.",byteData);
		cstr+=tstr;
		tstr.Empty();
		pbdata1++;
	}
		byteData = (int)(*pbdata1) & 0xff;
		tstr.Format("%d",byteData);
		cstr+=tstr;
		tstr.Empty();
	return cstr;
}

unsigned short ntohs(unsigned short netshort)
{
	return (((netshort<<8)&0xff00)+((netshort>>8)&0x00ff));
}

unsigned long ntohl(unsigned long netlong)
{
	return(((netlong>>24)&0x000000ff)+((netlong<<24)&0xff000000)+((netlong>>8)&0x0000ff00)+((netlong<<8)&0x00ff0000));
}
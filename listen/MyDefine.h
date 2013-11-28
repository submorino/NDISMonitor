#ifndef _MYDEFINE
#define MYDEFINE
#include "..\common\EthernetHeader.h"

CString GetNowTime();
CString QueryProtocol(BYTE iProtocol);
BYTE	QueryProtocolNum(CString csProtocol);
CString GetIPFromBytes(BYTE *pbyteData, int count);
CString BytestoString(BYTE *pbdata,int count);
unsigned short ntohs(unsigned short netshort);
unsigned long ntohl(unsigned long netlong);

#endif

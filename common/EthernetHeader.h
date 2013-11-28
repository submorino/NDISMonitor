#ifndef EHERNETHEADER
#define EHERNETHEADER

#define MAX_PROTO_TEXT_LEN	16		//子协议名称最大长度
#define MAX_PROTO_NUM 12			//子协议数量
#define MAX_PACKET_SIZE 65535
#define BUFFER_SIZE 2000

#ifdef SYSMODLE
UINT	Monitor_flag = 0;		// 监视标志，1->监视，0->不监视
UINT	Filt_flag = 0;			//过滤标志，1->过滤，0->不过滤

UINT Filter(UCHAR *pPacket);	// 过滤函数
VOID WritePacket2SharedMemory(UCHAR *pPacket,UINT packetsize); // 写数据包到共享内存函数

#else
#define IPPROTO_IP              0               /* dummy for IP */
#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_IGMP            2               /* internet group management protocol */
#define IPPROTO_GGP             3               /* gateway^2 (deprecated) */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_UDP             17              /* user datagram protocol */
#define IPPROTO_IDP             22              /* xns idp */
#define IPPROTO_ND              77              /* UNOFFICIAL net disk proto */

typedef struct _tagPROTOMAP
{
	BYTE  ProtoNum;
	char ProtoText[MAX_PROTO_TEXT_LEN];
}PROTOMAP;

static PROTOMAP ProtoMap[MAX_PROTO_NUM]=
{
	{ IPPROTO_IP   , "IP"  },
	{ IPPROTO_ICMP , "ICMP" }, 
	{ IPPROTO_IGMP , "IGMP" },
	{ IPPROTO_GGP  , "GGP" }, 
	{ IPPROTO_TCP  , "TCP" }, 
	{ IPPROTO_PUP  , "PUP" }, 
	{ IPPROTO_UDP  , "UDP" }, 
	{ IPPROTO_IDP  , "IDP" }, 
	{ IPPROTO_ND   , "NP"  },  
	{ NULL         , ""     }
};
#endif

typedef struct _TCP_HEADER					//定义TCP首部
{
	unsigned short th_sport;				//16位源端口
	unsigned short th_dport;				//16位目的端口
	unsigned long  th_seq;					//32位序列号
	unsigned long  th_ack;					//32位确认号
	unsigned char th_lenres;				//4位首部长度/6位保留字
	unsigned char th_flag;					//6位标志位
	unsigned short th_win;					//16位窗口大小
	unsigned short th_checksum;				//16位校验和
	unsigned short th_urp;					//16位紧急数据偏移量
}TCP_HEADER;

typedef struct _UDP_HEADER					//定义UDP首部
{
    unsigned short uh_sport;				//16位源端口
    unsigned short uh_dport;				//16位目的端口
    unsigned short uh_len;					//16位长度
    unsigned short uh_checksum;				//16位校验和
}UDP_HEADER;

typedef struct _ICMP_HEADER					//定义ICMP首部
{
	unsigned char   ih_type;				//8位类型
	unsigned char   ih_code;				//8位代码
	unsigned short ih_checksum;				//16位校验和 
}ICMP_HEADER;

typedef struct _IGMP_HEADER					//定义IGMP首部
{
	unsigned char   ih_type;				//8位类型
	unsigned char   ih_max_responsetime;	//8位最大响应时间
	unsigned short ih_checksum;				//16位校验和 
}IGMP_HEADER;


typedef struct _IP_HEADER
{
	unsigned char h_verlen;					//4位首部长度,4位IP版本号
	unsigned char tos;						//8位服务类型TOS
	unsigned short total_len;				//16位总长度（字节）
	unsigned short ident;					//16位标识
	unsigned short frag_and_flags;			//3位标志位和13位偏移
	unsigned char  ttl;						//8位生存时间 TTL
	unsigned char proto;					//8位协议 (1->ICMP, 2->IGMP, 6->TCP, 17->UDP)
	unsigned short checksum;				//16位IP首部校验和
	unsigned long sourceIP;					//32位源IP地址
	unsigned long destIP;					//32位目的IP地址
}IP_HEADER, *PIP_HEADER;

#endif
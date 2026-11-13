#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

const char *googleDnsServer = "8.8.8.8";
const int googleDnsPort = 53;
const int RecivePort = 10000;

typedef struct {
  unsigned short Id;
  // flags
  unsigned short Flags;
  unsigned short Que_Count;
  unsigned short Ans_Count;
  unsigned short Auth_count;
  unsigned short Add_count;

} Dns_header_;

typedef struct {
  unsigned short QueryType;
  unsigned short QueryClass;
} QueryStruct;

// for DNS request
Dns_header_ DnsReqHeader;
QueryStruct queryStruct;
char req_question[256];
int req_question_size;

// for recving DNS packet
Dns_header_ Recv_Header;
char recv_question[256];
QueryStruct RecvQueryStruct;
unsigned char recv_Rcode;

typedef struct {
  // we are skiping this
  //  unsigned short Name;
  //  unsigned short Type;
  //  unsigned short Class;
  //  unsigned int TTl;

  unsigned short RdLength;
  unsigned char *Rdata;
}Ans_Section;

Ans_Section *Recv_ans_section;

char DnsPacket[512];
int PacketSize;

struct sockaddr_in sendingTo;

struct sockaddr_in ReciveTo;
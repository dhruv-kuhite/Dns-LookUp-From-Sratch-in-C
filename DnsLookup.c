#include "DnsLookup.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

char DnsString[257] = "www.google.com";

void setDnsReqHeader() {

  memset(&DnsReqHeader, 0, sizeof(Dns_header_));
  DnsReqHeader.Id = htons(50);
  DnsReqHeader.Flags |= 1 << 8; // set RD to 1
  DnsReqHeader.Flags = htons(DnsReqHeader.Flags);
  DnsReqHeader.Que_Count = htons(1);
}

void FormatDnsLink() {

  int count = 0;
  int Qindex = 0;
  int Sindex = 0;

  for (int i = 0; i < (strlen(DnsString) + 1); i++) {

    if (i > 0 && DnsString[i - 1] == '.') {
      Sindex = i;
    }

    if (DnsString[i] == '.' || DnsString[i] == '\0') {
      req_question[Qindex] = count;
      Qindex++;
      memcpy(&req_question[Qindex], &DnsString[Sindex], count);
      Qindex += count;
      count = 0;
      continue;
    }
    count++;
  }

  req_question[Qindex] = '\0';

  req_question_size = Qindex;
}

void SetQueryStruct() {

  queryStruct.QueryClass = htons(1);
  queryStruct.QueryType = htons(1);
}

void CombineToMakeDnsReq() {

  setDnsReqHeader();
  FormatDnsLink();
  SetQueryStruct();
  PacketSize = 0;

  memset(DnsPacket, 0, sizeof(DnsPacket));
  memcpy(&DnsPacket[PacketSize], &DnsReqHeader, sizeof(Dns_header_));

  PacketSize += sizeof(Dns_header_);
  memcpy(&DnsPacket[PacketSize], req_question, strlen(req_question));
  PacketSize +=
      strlen(req_question) + 1; // end letter  should be 00 for it to work
                                // so +1 to memcopy with null being there
  memcpy(&DnsPacket[PacketSize], &queryStruct, sizeof(QueryStruct));
  PacketSize += sizeof(QueryStruct);
}

void setReciveToAndBind(int socketFd) {

  ReciveTo.sin_addr.s_addr = INADDR_ANY;
  ReciveTo.sin_port = htons(RecivePort);
  ReciveTo.sin_family = AF_INET;

  int check = bind(socketFd, (struct sockaddr *)&ReciveTo, sizeof(ReciveTo));

  if (check != 0) {
    printf("Bind Failed");
  }
}

void SendDnsPacket(int socketFd) {
  sendingTo.sin_family = AF_INET;
  sendingTo.sin_port = htons(googleDnsPort);

  inet_pton(AF_INET, googleDnsServer, &sendingTo.sin_addr.s_addr);

  int check = sendto(socketFd, DnsPacket, PacketSize, 0,
                     (struct sockaddr *)&sendingTo, sizeof(sendingTo));

  if (check == -1) {
    printf("Error Sending Data\n");
  }
}

void RecivingDnsPacket(int socketFd) {
  char buffer[1024];
  int sizeReciveTo = sizeof(ReciveTo);
  printf("Reciving..\n");
  int sizebuff = recvfrom(socketFd, buffer, sizeof(buffer), 0,
                          (struct sockaddr *)&ReciveTo, &sizeReciveTo);

  int index = 0;
  memcpy(&Recv_Header, &buffer[index], sizeof(Recv_Header));
  index += sizeof(Recv_Header);
  Recv_Header.Ans_Count = ntohs(Recv_Header.Ans_Count);
  memcpy(&recv_question, &buffer[index], req_question_size);
  index += req_question_size + 1;
  memcpy(&RecvQueryStruct, &buffer[index], sizeof(QueryStruct));
  index += sizeof(QueryStruct);

  Recv_ans_section =
      (Ans_Section *)malloc(sizeof(Ans_Section) * Recv_Header.Ans_Count);

  for (int i = 0; i < Recv_Header.Ans_Count; i++) {
    index += 10; // for skiping 10 bytes check DnsResolver.h line 40
    memcpy(&Recv_ans_section[i].RdLength, &buffer[index],
           sizeof(Recv_ans_section[i].RdLength));
    index += sizeof(Recv_ans_section[i].RdLength);
    Recv_ans_section[i].RdLength = ntohs(Recv_ans_section[i].RdLength);

    Recv_ans_section[i].Rdata =
        (unsigned char *)malloc(Recv_ans_section[i].RdLength);
    memcpy(Recv_ans_section[i].Rdata, &buffer[index],
           Recv_ans_section[i].RdLength);
    index += Recv_ans_section[i].RdLength;
  }
}

int CheckForSuccess() {

  recv_Rcode = ntohs(Recv_Header.Flags) & 15;

  printf("rcode : %d\n", recv_Rcode);
  switch (recv_Rcode) {

  case 0:
    printf("Success :)\n");
    return 0;
    break;

  case 1:
    printf("Format Error");
    return -1;
    break;

  case 2:
    printf("Server Failure");
    return -1;
    break;

  case 3:
    printf("Non-Existent Domain");
    return -1;
    break;

  case 4:
    printf("Not Implemented");
    return -1;
    break;

  case 5:
    printf("Refused");
    return -1;
    break;
  }

  return 0;
}

void printTheIpAddress() {

  char recv_ip[100];
  for (int i = 0; i < Recv_Header.Ans_Count; i++) {
    memset(recv_ip, 0, sizeof(recv_ip));
    inet_ntop(AF_INET, Recv_ans_section[i].Rdata, recv_ip, sizeof(recv_ip));
    printf("%d) IP : %s\n", i + 1, recv_ip);
  }
}

void FreeEveryThing() {

  for (int i = 0; i < Recv_Header.Ans_Count; i++) {
    free(Recv_ans_section[i].Rdata);
  }

  free(Recv_ans_section);
}

int main() {

  printf("Enter the Dns Name (Ex : www.example.com): ");
  scanf("%s", DnsString);
  int socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  CombineToMakeDnsReq();
  setReciveToAndBind(socketFd);
  SendDnsPacket(socketFd);
  RecivingDnsPacket(socketFd);
  if (CheckForSuccess() == -1) {
    return -1;
  }
  printTheIpAddress();
  FreeEveryThing();
  return 0;
}
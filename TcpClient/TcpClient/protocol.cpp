#include "protocol.h"

PDU *mkPDU(uint uiMsgLen)
{
    // 结构体的大小加上实际的消息长度（消息部分的长度不会计算进去）
    uint uiPDULen=sizeof(PDU)+uiMsgLen;
    PDU *pdu=(PDU*)malloc(uiPDULen);
    // 申请空间失败
    if(NULL==pdu)
    {
        exit(EXIT_FAILURE);
    }
    // 清0
    memset(pdu,0,uiPDULen);

    pdu->uiPDULen=uiPDULen;
    pdu->uiMsgLen=uiMsgLen;
    return pdu;
}

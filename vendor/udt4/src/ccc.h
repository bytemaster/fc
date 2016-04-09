/*****************************************************************************
Copyright (c) 2001 - 2009, The Board of Trustees of the University of Illinois.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the
  above copyright notice, this list of conditions
  and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the University of Illinois
  nor the names of its contributors may be used to
  endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu, last updated 02/28/2012
*****************************************************************************/


#ifndef __UDT_CCC_H__
#define __UDT_CCC_H__


#include "udt.h"
#include "packet.h"


class UDT_API CCC
{
friend class CUDT;

public:
   CCC();
   virtual ~CCC();

private:
   CCC(const CCC&);
   CCC& operator=(const CCC&) {return *this;}

public:

      // Functionality:
      //    Callback function to be called (only) at the start of a UDT connection.
      //    note that this is different from CCC(), which is always called.
      // Parameters:
      //    None.
      // Returned value:
      //    None.

   virtual void init() {}

      // Functionality:
      //    Callback function to be called when a UDT connection is closed.
      // Parameters:
      //    None.
      // Returned value:
      //    None.

   virtual void close() {}

      // Functionality:
      //    Callback function to be called when an ACK packet is received.
      // Parameters:
      //    0) [in] ackno: the data sequence number acknowledged by this ACK.
      // Returned value:
      //    None.

   virtual void onACK(int32_t) {}

      // Functionality:
      //    Callback function to be called when a loss report is received.
      // Parameters:
      //    0) [in] losslist: list of sequence number of packets, in the format describled in packet.cpp.
      //    1) [in] size: length of the loss list.
      // Returned value:
      //    None.

   virtual void onLoss(const int32_t*, int) {}

      // Functionality:
      //    Callback function to be called when a timeout event occurs.
      // Parameters:
      //    None.
      // Returned value:
      //    None.

   virtual void onTimeout() {}

      // Functionality:
      //    Callback function to be called when a data is sent.
      // Parameters:
      //    0) [in] seqno: the data sequence number.
      //    1) [in] size: the payload size.
      // Returned value:
      //    None.

   virtual void onPktSent(const CPacket*) {}

      // Functionality:
      //    Callback function to be called when a data is received.
      // Parameters:
      //    0) [in] seqno: the data sequence number.
      //    1) [in] size: the payload size.
      // Returned value:
      //    None.

   virtual void onPktReceived(const CPacket*) {}

      // Functionality:
      //    Callback function to Process a user defined packet.
      // Parameters:
      //    0) [in] pkt: the user defined packet.
      // Returned value:
      //    None.

   virtual void processCustomMsg(const CPacket*) {}

protected:

      // Functionality:
      //    Set periodical acknowldging and the ACK period.
      // Parameters:
      //    0) [in] msINT: the period to send an ACK.
      // Returned value:
      //    None.

   void setACKTimer(int msINT);

      // Functionality:
      //    Set packet-based acknowldging and the number of packets to send an ACK.
      // Parameters:
      //    0) [in] pktINT: the number of packets to send an ACK.
      // Returned value:
      //    None.

   void setACKInterval(int pktINT);

      // Functionality:
      //    Set RTO value.
      // Parameters:
      //    0) [in] msRTO: RTO in macroseconds.
      // Returned value:
      //    None.

   void setRTO(int usRTO);

      // Functionality:
      //    Send a user defined control packet.
      // Parameters:
      //    0) [in] pkt: user defined packet.
      // Returned value:
      //    None.

   void sendCustomMsg(CPacket& pkt) const;

      // Functionality:
      //    retrieve performance information.
      // Parameters:
      //    None.
      // Returned value:
      //    Pointer to a performance info structure.

   const CPerfMon* getPerfInfo();

      // Functionality:
      //    Set user defined parameters.
      // Parameters:
      //    0) [in] param: the paramters in one buffer.
      //    1) [in] size: the size of the buffer.
      // Returned value:
      //    None.

   void setUserParam(const char* param, int size);

private:
   void setMSS(int mss);
   void setMaxCWndSize(int cwnd);
   void setBandwidth(int bw);
   void setSndCurrSeqNo(int32_t seqno);
   void setRcvRate(int rcvrate);
   void setRTT(int rtt);

protected:
   const int32_t& m_iSYNInterval;	// UDT constant parameter, SYN

   double m_dPktSndPeriod;              // Packet sending period, in microseconds
   double m_dCWndSize;                  // Congestion window size, in packets

   int m_iBandwidth;			// estimated bandwidth, packets per second
   double m_dMaxCWndSize;               // maximum cwnd size, in packets

   int m_iMSS;				// Maximum Packet Size, including all packet headers
   int32_t m_iSndCurrSeqNo;		// current maximum seq no sent out
   int m_iRcvRate;			// packet arrive rate at receiver side, packets per second
   int m_iRTT;				// current estimated RTT, microsecond

   char* m_pcParam;			// user defined parameter
   int m_iPSize;			// size of m_pcParam

private:
   UDTSOCKET m_UDT;                     // The UDT entity that this congestion control algorithm is bound to

   int m_iACKPeriod;                    // Periodical timer to send an ACK, in milliseconds
   int m_iACKInterval;                  // How many packets to send one ACK, in packets

   bool m_bUserDefinedRTO;              // if the RTO value is defined by users
   int m_iRTO;                          // RTO value, microseconds

   CPerfMon m_PerfInfo;                 // protocol statistics information
};

class CCCVirtualFactory
{
public:
   virtual ~CCCVirtualFactory() {}

   virtual CCC* create() = 0;
   virtual CCCVirtualFactory* clone() = 0;
};

template <class T>
class CCCFactory: public CCCVirtualFactory
{
public:
   virtual ~CCCFactory() {}

   virtual CCC* create() {return new T;}
   virtual CCCVirtualFactory* clone() {return new CCCFactory<T>;}
};

class CUDTCC: public CCC
{
public:
   CUDTCC();

public:
   virtual void init();
   virtual void onACK(int32_t);
   virtual void onLoss(const int32_t*, int);
   virtual void onTimeout();

private:
   int m_iRCInterval;			// UDT Rate control interval
   uint64_t m_LastRCTime;		// last rate increase time
   bool m_bSlowStart;			// if in slow start phase
   int32_t m_iLastAck;			// last ACKed seq no
   bool m_bLoss;			// if loss happened since last rate increase
   int32_t m_iLastDecSeq;		// max pkt seq no sent out when last decrease happened
   double m_dLastDecPeriod;		// value of pktsndperiod when last decrease happened
   int m_iNAKCount;                     // NAK counter
   int m_iDecRandom;                    // random threshold on decrease by number of loss events
   int m_iAvgNAKNum;                    // average number of NAKs per congestion
   int m_iDecCount;			// number of decreases in a congestion epoch
};

#endif

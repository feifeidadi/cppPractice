#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <arpa/inet.h>

#include "ouchProtocol.H"

#ifdef DEBUG
    #define OUTPUT(x) do       { std::cout << x << std::endl; } while(0)
    #define ERROR_OUTPUT(x) do { std::cerr << x << std::endl; } while(0)
#else
    #define OUTPUT(x) do { } while(0)
    #define ERROR_OUTPUT(x)  do { } while(0)
#endif

// Internal data structure for packet capture statistics
struct PkgCaptureStats
{
  uint32_t numAcceptedMsgs = 0;
  uint32_t numSystemEvent = 0;
  uint32_t numReplacedMsgs = 0;
  uint32_t numCanceledMsgs = 0;
  uint32_t numExecutedMsgs = 0;
  uint32_t numExecutedShares = 0;

  void increaseSystemEvents() { ++numSystemEvent;  }
  void increaseAcceptedMsgs() { ++numAcceptedMsgs; }
  void increaseReplacedMsgs() { ++numReplacedMsgs; }
  void increaseExecutedMsgs() { ++numExecutedMsgs; }
  void increaseCanceledMsgs() { ++numCanceledMsgs; }
  void increaseExecutedShares(uint32_t executedShares) { numExecutedShares += executedShares; }

  void increaseNumMsgs(const char msgType)
  {
    switch (msgType)
    {
      case OuchMessageType::ACCEPTED:
        increaseAcceptedMsgs();
        break;
      case OuchMessageType::REPLACED:
        increaseReplacedMsgs();
        break;
      case OuchMessageType::EXECUTED:
        increaseExecutedMsgs();
        break;
      case OuchMessageType::CANCELED:
        increaseCanceledMsgs();
        break;
      case OuchMessageType::SYSTEM_EVENT:
        increaseSystemEvents();
        break;
      default:
        break;
    }
  }

  void printInfo() const
  {
    std::cout << " Accepted: "     << getNumAcceptedMsgs() << " messages" << std::endl;
    std::cout << " System Event: " << getNumSystemEvent()  << " messages" << std::endl;
    std::cout << " Replaced: "     << getNumReplacedMsgs() << " messages" << std::endl;
    std::cout << " Canceled: "     << getNumCanceledMsgs() << " messages" << std::endl;
    std::cout << " Executed: "     << getNumExecutedMsgs() << " messages: ";
    std::cout << getNumExecutedShares() << " executed shares" << std::endl;
  }

  uint32_t getNumSystemEvent()    const { return numSystemEvent;  }
  uint32_t getNumAcceptedMsgs()   const { return numAcceptedMsgs; }
  uint32_t getNumReplacedMsgs()   const { return numReplacedMsgs; }
  uint32_t getNumCanceledMsgs()   const { return numCanceledMsgs; }
  uint32_t getNumExecutedMsgs()   const { return numExecutedMsgs; }
  uint32_t getNumExecutedShares() const { return numExecutedShares; }
};

class ouchParser
{
public:
  ouchParser()  {}
  ~ouchParser() {}

  int  parsePacketFile(const std::string& filename);
  void printStats() const;

private:
  int  parsePacketFile();
  void loadPacketFileIntoMap();
  void initPkgCaptureStats();
  void parseStreams();
  void parsePackets(const Packets& packets, PkgCaptureStats& stat);
  void parsePacket(const Packet& ouchMessage, PkgCaptureStats& stat);
  void printOuchMsgState(const OuchMessageState ouchMsgState);

  template <typename T>
  void parseOuchMessage(const T ouchMessage, PkgCaptureStats& stat);

  template<typename T>
  bool tryParse(const Packet& ouchMessage, PkgCaptureStats& stat);

  inline uint16_t getOuchMsgLength(const Packet& packet);
  void getPacketState(const Packet& packet, OuchMessageState& ouchMsgState);
  Packet& combineTwoPackets(const Packet first, const Packet second);
  void updateOuchMsgState(const OuchMessageState newOuchsgState, OuchMessageState& ouchMsgState);

  std::ifstream m_file;
  Streams m_streams;
  Stats m_stats;
  PkgCaptureStats m_allPkgCaptureStats{}; // Statistics of all streams packets
};

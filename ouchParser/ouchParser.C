#include "ouchParser.H"
#include "hexDump.H"

void ouchParser::updateOuchMsgState(const OuchMessageState newOuchMsgState, OuchMessageState& ouchMsgState)
{
  static const std::unordered_map<OuchMessageState, std::string_view> stateMsgMap = {
    {OuchMessageState::COMPLETE, "--- Full OUCH protocol message ---\n"},
    {OuchMessageState::PARTIAL, "--- Part (1/2) OUCH protocol message ---\n"},
    {OuchMessageState::PARTIAL_TO_FULL, "--- Part (2/2) OUCH protocol message ---\n"},
    {OuchMessageState::EMPTY, "--- Empty OUCH protocol message ---\n"},
  };

  const auto& it = stateMsgMap.find(newOuchMsgState);
  if (it != stateMsgMap.end())
  {
    OUTPUT(it->second);
  }

  ouchMsgState = newOuchMsgState;
}

uint16_t ouchParser::getOuchMsgLength(const Packet& packet)
{
  uint16_t OuchMsgLenthBE;
  std::memcpy(&OuchMsgLenthBE, packet.data(), 2);
  return ntohs(OuchMsgLenthBE); // Convert Big Endian -> Host byte order
}

void ouchParser::printOuchOrdersInfo(const uint32_t accepted, const uint32_t systemEvent,
                         const uint32_t replaced, const uint32_t canceled,
                         const uint32_t executed, const uint32_t executedShares)
{
  std::cout << " Accepted: "     << accepted << " messages" << std::endl;
  std::cout << " System Event: " << systemEvent << " messages" << std::endl;
  std::cout << " Replaced: "     << replaced << " messages" << std::endl;
  std::cout << " Canceled: "     << canceled << " messages" << std::endl;
  std::cout << " Executed: "     << executed << " messages: ";
  std::cout << executedShares    << " executed shares" << std::endl;
}

void ouchParser::printStats()
{
  uint32_t totalSystemEvent{0}, totalAccepted{0}, totalReplaced{0}, totalCanceled{0}, totalExecuted{0}, totalExecutedShares{0};
  for (const auto& [streamId, stat] : m_stats)
  {
      std::cout << "Stream " << std::dec << streamId << std::endl;
      printOuchOrdersInfo(stat.getNumAcceptedMsgs(), stat.getNumSystemEvent(),
                          stat.getNumReplacedMsgs(), stat.getNumCanceledMsgs(),
                          stat.getNumExecutedMsgs(), stat.getExecutedShares());

      totalSystemEvent += stat.getNumSystemEvent();
      totalAccepted += stat.getNumAcceptedMsgs();
      totalReplaced += stat.getNumReplacedMsgs();
      totalCanceled += stat.getNumCanceledMsgs();
      totalExecuted += stat.getNumExecutedMsgs();
      totalExecutedShares += stat.getNumExecutedShares();
  }

  std::cout << "Totals:" << std::endl;
  printOuchOrdersInfo(totalAccepted, totalSystemEvent, totalReplaced,
                      totalCanceled, totalExecuted, totalExecutedShares);
}

template <typename T>
void ouchParser::parseOuchMessage(const T ouchMessage, PkgCaptureStats& stat) 
{
  const auto msgType = ouchMessage->getMessageType();
  OUTPUT("--- Ouch " << ouchMessage->getMessageTypeStr(static_cast<OuchMessageType>(msgType)) << " message ---\n");
  if (msgType == OuchMessageType::EXECUTED)
  {
    OUTPUT("---  " << ouchMessage->getShares() << " shares executed ---\n");
    stat.increaseExecutedShares(ouchMessage->getShares());
  }
  stat.increaseNumMsgs(msgType);
}

template<typename T>
bool ouchParser::tryParse(const Packet& ouchMessage, PkgCaptureStats& stat) {
    if (ouchMessage[3] == T::MSG_TYPE && ouchMessage.size() == sizeof(T))
    {
        parseOuchMessage(reinterpret_cast<const T*>(ouchMessage.data()), stat);
        hexDump(ouchMessage.data(), ouchMessage.size());
        return true;
    }

    return false;
}

void ouchParser::parsePacket(const Packet& ouchMessage, PkgCaptureStats& stat)
{
    bool parsed = tryParse<OUCHAcceptedMessage>(ouchMessage, stat) ||
                  tryParse<OUCHCanceledMessage>(ouchMessage, stat) ||
                  tryParse<OUCHExecutedMessage>(ouchMessage, stat) ||
                  tryParse<OUCHReplacedMessage>(ouchMessage, stat) ||
                  tryParse<OUCHSystemEventMessage>(ouchMessage, stat);

    if (!parsed)
    {
      ERROR_OUTPUT("--- ERROR: Unexpected OUCH message: size = " << ouchMessage.size() << ", msgType = " << ouchMessage[3] << " ---\n");
    }
}

/*
 * This is the key function to determine if the packet is full or parital.
 * The packet is ready to parse only if ouchMsgState is PARTIAL_TO_FULL or COMPLETE
 * Empty packet will be ignored.
 * It will return PARTIAL_TO_FULL if the previous packet is a partial OUCH msg
 * Ensure packet size >= 2 before getting the msg length is mandatory, because the first two bytes contain the OUCH Message Length field
 */
void ouchParser::getPacketState(const Packet& packet, OuchMessageState& ouchMsgState)
{
  const auto packetSize = packet.size();
  if (packetSize == 0) // Ignore empty packet
  {
    updateOuchMsgState(OuchMessageState::EMPTY, ouchMsgState);
    return;
  }

  if (OuchMessageState::PARTIAL == ouchMsgState) // Previous packet is a partial OUCH message
  {
    updateOuchMsgState(OuchMessageState::PARTIAL_TO_FULL, ouchMsgState); // Packet 1/2 + Packet 2/2 => Full OUCH msg
    return;
  }

  if (packetSize >= 2)
  {
    const auto ouchMsgLenth = getOuchMsgLength(packet);
    if (ouchMsgLenth == (packetSize - 2)) // The packet size excludes the OUCH Message Length field
    {
      updateOuchMsgState(OuchMessageState::COMPLETE, ouchMsgState);
      return;
    }
  }

  updateOuchMsgState(OuchMessageState::PARTIAL, ouchMsgState);
}

Packet& ouchParser::combineTwoPackets(const Packet first, const Packet second)
{
  static Packet combinedPacket;
  combinedPacket.clear();
  combinedPacket.reserve(first.size() + second.size());
  combinedPacket.insert(combinedPacket.end(), first.begin(), first.end());
  combinedPacket.insert(combinedPacket.end(), second.begin(), second.end());

  const auto ouchMsgLenth = getOuchMsgLength(combinedPacket);
  OUTPUT("--- Combined packet size: " << combinedPacket.size() << " bytes) ---\n");

  assert(ouchMsgLenth == (combinedPacket.size()-2) && "Packet size and OUCH message size mismatch.");

  return combinedPacket;
}
/*
 * Go through all the packets in one stream and parse it 
 * when it's ready (ouchMsgState in [OuchMessageState::COMPLETE, OuchMessageState::PARTIAL_TO_FULL])
 */
void ouchParser::parsePackets(const uint16_t streamId, const Packets& packets)
{
  OuchMessageState ouchMsgState{OuchMessageState::UNKNOWN};
  for (size_t i = 0; i < packets.size(); ++i) 
  {
    OUTPUT("--- Packet " << i << " (" << packets[i].size() << " bytes) ---\n");
    getPacketState(packets[i], ouchMsgState);
    if (ouchMsgState == OuchMessageState::COMPLETE) // Full OUCH protocol message
    {
      parsePacket(packets[i], m_stats[streamId]);
      ouchMsgState = OuchMessageState::UNKNOWN; // Rest ouchMsgState
    }
    else if (ouchMsgState == OuchMessageState::PARTIAL_TO_FULL)
    {
      // Append current packet at the end of previous one to get a full OUCH message
      const auto& pkt = combineTwoPackets(packets[i-1], packets[i]);
      parsePacket(pkt, m_stats[streamId]);
      ouchMsgState = OuchMessageState::UNKNOWN; // Rest ouchMsgState
    }
  }
}

// Parse streams and save all the OUCH packets information into m_stats
void ouchParser::parseStreams()
{
  for (const auto& [streamId, packets] : m_streams) 
  {
    OUTPUT("===== Stream " << streamId << " (" << packets.size() << " packets) =====\n");
    parsePackets(streamId, packets);
  }
}

// Read packet capture file and store all the packets into m_streams
void ouchParser::loadPacketFileIntoMap()
{
  uint16_t streamIdBE{0};
  uint32_t payloadLengthBE{0};
  while (m_file.read(reinterpret_cast<char*>(&streamIdBE), 2) &&    // The first 2 bytes - Stream Identifier
         m_file.read(reinterpret_cast<char*>(&payloadLengthBE), 4)) // The next 4 bytes  - Packet Length
  {
      // Convert Big Endian -> Host byte order
      const uint16_t streamId = ntohs(streamIdBE);
      const uint32_t payload_length = ntohl(payloadLengthBE);

      // Read payload (OUCH message)
      Packet payload(payload_length);
      if (!m_file.read(reinterpret_cast<char*>(payload.data()), payload_length)) {
          std::cerr << "File truncated or corrupted\n";
          break;
      }

      // Store payload (OUCH message) into streams map
      m_streams[streamId].push_back(std::move(payload));
  }
}

void ouchParser::initPkgCaptureStats()
{
  OUTPUT("Number of streams: " << m_streams.size() << "\n");
  for (const auto& [streamId, packets] : m_streams)
  {
    OUTPUT("Stream " << streamId << " -> " << packets.size() << " packets\n");
    // Initialize PkgCaptureStats for each stream, all zero at this moment
    m_stats.emplace(streamId, PkgCaptureStats{});
  }
}

// Loading packet capture file into memory and parsing
int ouchParser::parsePacketFile()
{
  loadPacketFileIntoMap();
  initPkgCaptureStats();
  parseStreams();
  return 0;
}

int ouchParser::parsePacketFile(const std::string& filename)
{
  m_file.open(filename, std::ios::binary);
  if (!m_file)
  {
    std::cerr << "Failed to open file: " << filename << "\n";
    return -1;
  }

  return parsePacketFile();
}

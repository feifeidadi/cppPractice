#include "ouchParser.H"
#include "hexDump.H"

void ouchParser::printOuchMsgState([[maybe_unused]]const OuchMessageState ouchMsgState)
{
#ifdef DEBUG
  static const std::string msg{"OUCH protocol message"};
  static const std::unordered_map<OuchMessageState, std::string> stateMsgMap = {
    {OuchMessageState::COMPLETE, "Full " + msg},
    {OuchMessageState::PARTIAL, "Part (1/2) "  + msg},
    {OuchMessageState::PARTIAL_TO_FULL, "Part (2/2) " + msg},
  };

  const auto& it = stateMsgMap.find(ouchMsgState);
  if (it != stateMsgMap.end())
  {
    OUTPUT(it->second);
  }
#endif
}

void ouchParser::updateOuchMsgState(const OuchMessageState newOuchMsgState, OuchMessageState& ouchMsgState)
{
  ouchMsgState = newOuchMsgState;
  printOuchMsgState(ouchMsgState);
}

uint16_t ouchParser::getOuchMsgLength(const Packet& packet)
{
  uint16_t OuchMsgLenthBE;
  std::memcpy(&OuchMsgLenthBE, packet.data(), 2);
  return ntohs(OuchMsgLenthBE); // Convert Big Endian -> Host byte order
}

void ouchParser::printStats() const
{
  for (const auto& [streamId, stat] : m_stats)
  {
    std::cout << "Stream " << std::dec << streamId << std::endl;
    stat.printInfo();
  }

  std::cout << "Totals:" << std::endl;
  m_allPkgCaptureStats.printInfo();
}

template <typename T>
void ouchParser::parseOuchMessage(const T ouchMessage, PkgCaptureStats& stat)
{
  const auto msgType = ouchMessage->getMessageType();
  OUTPUT("Ouch " << ouchMessage->getMessageTypeStr(static_cast<OuchMessageType>(msgType)) << " message");
  if (msgType == OuchMessageType::EXECUTED)
  {
    OUTPUT(ouchMessage->getShares() << " shares executed");
    stat.increaseExecutedShares(ouchMessage->getShares());
    m_allPkgCaptureStats.increaseExecutedShares(ouchMessage->getShares());
  }
  stat.increaseNumMsgs(msgType);
  m_allPkgCaptureStats.increaseNumMsgs(msgType);
}

template<typename T>
bool ouchParser::tryParse(const Packet& ouchMessage, PkgCaptureStats& stat)
{
  if (ouchMessage[3] == T::MSG_TYPE && ouchMessage.size() == sizeof(T))
  {
    parseOuchMessage(reinterpret_cast<const T*>(ouchMessage.data()), stat);
    hexDump(ouchMessage.data(), ouchMessage.size());
    return true;
  }

  return false;
}

void ouchParser::parseFullPacket(const Packet& ouchMessage, PkgCaptureStats& stat)
{
  bool parsed = tryParse<OUCHAcceptedMessage>(ouchMessage, stat) ||
                tryParse<OUCHCanceledMessage>(ouchMessage, stat) ||
                tryParse<OUCHExecutedMessage>(ouchMessage, stat) ||
                tryParse<OUCHReplacedMessage>(ouchMessage, stat) ||
                tryParse<OUCHSystemEventMessage>(ouchMessage, stat);

  if (!parsed)
  {
    ERROR_OUTPUT("ERROR: Unexpected OUCH message: size = " << ouchMessage.size() << ", msgType = " << ouchMessage[3]);
  }
}

/*
 * This is the key function to determine if the packet is full or parital.
 * The packet is ready to parse only if ouchMsgState is PARTIAL_TO_FULL or COMPLETE
 * It will return PARTIAL_TO_FULL if the previous packet is a partial OUCH msg
 * Ensure packet size >= 2 before getting the msg length is mandatory, because the first two bytes contain the OUCH Message Length field
 * Empty packet has already been filtered.
 */
void ouchParser::getPacketState(const Packet& packet, OuchMessageState& ouchMsgState)
{
  if (OuchMessageState::PARTIAL == ouchMsgState) // Previous packet is a partial OUCH message
  {
    updateOuchMsgState(OuchMessageState::PARTIAL_TO_FULL, ouchMsgState); // Packet 1/2 + Packet 2/2 => Full OUCH msg
    return;
  }

  const auto packetSize = packet.size();
  if (packetSize >= 2 &&
      getOuchMsgLength(packet) == (packetSize - 2)) // The packet size excludes the OUCH Message Length field (2 bytes)
  {
    updateOuchMsgState(OuchMessageState::COMPLETE, ouchMsgState); // The packet contains a complete ouch message
    return;
  }

  // Default is partial if ouchMsgState not in [EMPTY, PARTIAL_TO_FULL, COMPLETE]
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
  OUTPUT("Combined packet size: " << combinedPacket.size() << " bytes)");

  assert(ouchMsgLenth == (combinedPacket.size() - 2) && "Packet size and OUCH message size mismatch.");

  return combinedPacket;
}

/*
 * Get packet state and parse it if ouchMsgState in [OuchMessageState::COMPLETE, OuchMessageState::PARTIAL_TO_FULL]
 */
void ouchParser::parsePacket(const Packet& packet, PkgCaptureStats& stat)
{
  static OuchMessageState ouchMsgState{OuchMessageState::UNKNOWN};
  static Packet *partialPacket{nullptr};

  getPacketState(packet, ouchMsgState);
  if (ouchMsgState == OuchMessageState::COMPLETE ||
      ouchMsgState == OuchMessageState::PARTIAL_TO_FULL)
  {
    const auto& pkt = partialPacket == nullptr ? packet /* COMPLETE OUCH message */ :
                                                 // Combine previous previous and current packet to get a full OUCH message
                                                 combineTwoPackets(*partialPacket, packet);
    parseFullPacket(pkt, stat);
    ouchMsgState = OuchMessageState::UNKNOWN; // Rest ouchMsgState
    partialPacket = nullptr; // Actually it's only needed if ouchMsgState == OuchMessageState::PARTIAL_TO_FULL
  }
  else if (ouchMsgState == OuchMessageState::PARTIAL)
  {
    // Save partial packet address, it will be used to combine with the next packet to get a full packet
    partialPacket = const_cast<Packet*>(&packet);
  }
}

/*
 * Go through all the packets in one stream and parse it
 */
void ouchParser::parsePackets(const Packets& packets, PkgCaptureStats& stat)
{
  for (size_t i = 0; i < packets.size(); ++i)
  {
    if (packets[i].size() == 0)
    {
      OUTPUT("Empty packet (0 bytes), skipped");
      continue;
    }
    OUTPUT("Packet " << i << " (" << packets[i].size() << " bytes)");
    parsePacket(packets[i], stat);
  }
}

// Parse streams and save all the OUCH packets information into m_stats
void ouchParser::parseStreams()
{
  for (const auto& [streamId, packets] : m_streams)
  {
    OUTPUT("Stream " << streamId << " (" << packets.size() << " packets)");
    parsePackets(packets, m_stats[streamId]);
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
  OUTPUT("Number of streams: " << m_streams.size());
  for (const auto& [streamId, packets] : m_streams)
  {
    OUTPUT("Stream " << streamId << " -> " << packets.size() << " packets");
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

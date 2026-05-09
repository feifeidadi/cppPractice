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

  if (const auto& it = stateMsgMap.find(ouchMsgState); it != stateMsgMap.end())[[likely]]
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
  std::memcpy(&OuchMsgLenthBE, packet.data(), OUCH_MSG_LENGTH_FIELD_SIZE);
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

void ouchParser::increaseExecutedShares(PkgCaptureStats& stat, uint32_t execShares)
{
  OUTPUT(execShares << " shares executed");
  stat.increaseExecutedShares(execShares);
  m_allPkgCaptureStats.increaseExecutedShares(execShares);
}

void ouchParser::increaseNumMsgs(PkgCaptureStats& stat, char msgType)
{
  stat.increaseNumMsgs(msgType);
  m_allPkgCaptureStats.increaseNumMsgs(msgType);
}

template <typename T>
void ouchParser::parseOuchMessage(const T ouchMessage, PkgCaptureStats& stat)
{
  const auto msgType = ouchMessage->getMessageType();
  OUTPUT("Ouch " << ouchMessage->getMessageTypeStr(static_cast<OuchMessageType>(msgType)) << " message");
  if (isExecutedOuchMessage(msgType))
  {
    increaseExecutedShares(stat, ouchMessage->getShares());
  }
  increaseNumMsgs(stat, msgType);
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

bool ouchParser::isCompletePacket(const Packet& packet)
{
  const auto packetSize = packet.size();
  if (packetSize >= OUCH_MSG_LENGTH_FIELD_SIZE &&
      getOuchMsgLength(packet) == (packetSize - OUCH_MSG_LENGTH_FIELD_SIZE)) // The packet size excludes the OUCH Message Length field (2 bytes)
  {
    return true;
  }

  return false;
}

/*
 * This is the key function to determine if the packet is full or parital.
 * Ensure packet size >= 2 before getting the msg length is mandatory, because the first two bytes contain the OUCH Message Length field
 * Empty packet has already been filtered.
 *
 * Input: packet, ouchMsgState (indicating if previous packet is a partial ouch message)
 * Output:
 *   ouchMsgState:
 *     PARTIAL_TO_FULL or COMPLETE: packet is ready to parse
 *     PARTIAL                    : Packet 1/2, wait for packet 2/2
 */
void ouchParser::getPacketState(const Packet& packet, OuchMessageState& ouchMsgState)
{
  if (OuchMessageState::PARTIAL == ouchMsgState) // Previous packet is a partial OUCH message
  {
    updateOuchMsgState(OuchMessageState::PARTIAL_TO_FULL, ouchMsgState); // Packet 1/2 + Packet 2/2 => Full OUCH msg
    return;
  }

  if (isCompletePacket(packet))
  {
    updateOuchMsgState(OuchMessageState::COMPLETE, ouchMsgState);
    return;
  }

  // Default is partial if ouchMsgState is not PARTIAL_TO_FULL or COMPLETE
  updateOuchMsgState(OuchMessageState::PARTIAL, ouchMsgState);
}

const Packet& ouchParser::combineTwoPackets(const Packet& first, const Packet& second)
{
  static Packet combinedPacket(MAX_OUCH_MSG_SIZE*2);
  combinedPacket.clear();
  combinedPacket.insert(combinedPacket.end(), first.begin(), first.end());
  combinedPacket.insert(combinedPacket.end(), second.begin(), second.end());

  const auto ouchMsgLenth = getOuchMsgLength(combinedPacket);
  OUTPUT("Combined packet size: " << combinedPacket.size() << " bytes)");

  assert(ouchMsgLenth == (combinedPacket.size() - OUCH_MSG_LENGTH_FIELD_SIZE) && "Packet size and OUCH message size mismatch.");

  return combinedPacket;
}

/*
 * Return a full packet if ouchMsgState is COMPLETE or PARTIAL_TO_FULL
 * Otherwise return an empty packet (ouchMsgState == PARTIAL)
 */
const Packet& ouchParser::getFullPkt(const Packet& packet, const std::optional<std::reference_wrapper<Packet>> partialPacket, OuchMessageState& ouchMsgState)
{
  static Packet emptyPkt{};
  getPacketState(packet, ouchMsgState);
  if (ouchMsgState == OuchMessageState::COMPLETE)
  {
    return packet; // It's already a complete packet
  }

  if (ouchMsgState == OuchMessageState::PARTIAL_TO_FULL && partialPacket)
  {
    return combineTwoPackets(partialPacket->get(), packet); // Combine previous and current packet to get a full OUCH packet
  }

  return emptyPkt;
}

void ouchParser::parsePacket(const Packet& packet, PkgCaptureStats& stat)
{
  static OuchMessageState ouchMsgState{OuchMessageState::UNKNOWN};
  static std::optional<std::reference_wrapper<Packet>> partialPacket;

  const auto pkt = getFullPkt(packet, partialPacket, ouchMsgState);
  if (not pkt.empty())
  {
    parseFullPacket(pkt, stat);
    ouchMsgState = OuchMessageState::UNKNOWN; // Rest ouchMsgState
    partialPacket.reset(); // Actually it's only needed if ouchMsgState == OuchMessageState::PARTIAL_TO_FULL
    return;
  }

  // Must be a partial packet (ouchMsgState == OuchMessageState::PARTIAL), save it in the reference
  partialPacket = const_cast<Packet&>(packet);
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

/*
 *  Return values:
 *    -1: Failed to save the packet into map
 *     0: Success
 */
int ouchParser::savePacketIntoMap(std::ifstream& ifs, const uint16_t streamId, const uint32_t packetLength)
{
  Packet pkt(packetLength);
  if (!ifs.read(reinterpret_cast<char*>(pkt.data()), packetLength)) [[unlikely]] // Read OUCH message packet
  {
    std::cerr << "File truncated or corrupted\n";
    return -1;
  }

  // Store packet (OUCH message) into streams map
  m_streams[streamId].push_back(std::move(pkt));

  return 0;
}

// Read packet capture file and store all the packets into m_streams
void ouchParser::loadPacketFileIntoMap()
{
  uint16_t streamIdBE{0};
  uint32_t payloadLengthBE{0};
  while (m_file.read(reinterpret_cast<char*>(&streamIdBE), STREAM_IDENTIFIER_FIELD_SIZE) && // The first 2 bytes - Stream Identifier
         m_file.read(reinterpret_cast<char*>(&payloadLengthBE), PACKET_LENGTH_FIELD_SIZE))  // The next 4 bytes  - Packet Length
  {
    // ntohs()/ntohl() APIs Convert Big Endian -> Host byte order
    if (savePacketIntoMap(m_file, ntohs(streamIdBE), ntohl(payloadLengthBE)) < 0)
    {
      break; // Something wrong in the packet file
    }
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

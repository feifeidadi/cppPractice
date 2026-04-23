#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <cassert>

enum OuchMessageType
{
  SYSTEM_EVENT = 'S',
  ACCEPTED     = 'A',
  REPLACED     = 'U',
  EXECUTED     = 'E',
  CANCELED     = 'C',
  INVALID      = -1,
};

enum OuchMessageState
{
  EMPTY           = 0,
  COMPLETE        = 1,
  PARTIAL         = 2,
  PARTIAL_TO_FULL = 3,
  UNKNOWN         = -1,
};

using Packet  = std::vector<uint8_t>;
using Packets = std::vector<Packet>;
// streamId -> Packets
using Streams = std::unordered_map<uint16_t, Packets>;

struct PkgCaptureStats; 
// streamId -> packet capture staticstics
using Stats   = std::unordered_map<uint16_t, PkgCaptureStats>;

#pragma pack(push, 1)  // Ensure no padding between fields
struct OUCHSystemEventMessage
{
    static constexpr OuchMessageType MSG_TYPE = OuchMessageType::SYSTEM_EVENT;

    uint16_t message_length;        // OUCH Message Length (excluding this field) [expect 11]
    char packet_type;               // OUCH Packet Type  - Always 'S' indicating Sequenced
    char message_type;              // OUCH Message Type - Always 'S' indicating System Event
    uint64_t timestamp;
    char event_enumeration;

    char getMessageType() const { return message_type;  }
    uint32_t getShares()  const { return 0; }
};
static_assert(sizeof(OUCHSystemEventMessage) == 13, "OUCHSystemEventMessage size must be 13 bytes");

// Big Endian for all integers in the packet capture file
struct OUCHAcceptedMessage
{
    static constexpr OuchMessageType MSG_TYPE = OuchMessageType::ACCEPTED;

    uint16_t message_length;        // OUCH Message Length (excluding this field) [expect 66]
    char packet_type;               // OUCH Packet Type  - Always 'S' indicating Sequenced
    char message_type;              // OUCH Message Type - Always 'A' indicating Accepted
    uint64_t timestamp;
    char order_token[14];
    char side;
    uint32_t shares;
    char symbol[8];
    uint32_t price;                 // Price (x 10,000)
    uint32_t time_in_force;
    char firm[4];
    char display;
    uint64_t order_ref_number;
    char order_capacity;
    char intermarket_sweep;
    uint32_t min_quantity;
    char cross_type;
    char order_state;

    uint32_t getShares()      const { return ntohl(shares); }
    char     getMessageType() const { return message_type;  }
};
static_assert(sizeof(OUCHAcceptedMessage) == 68, "OUCHAcceptedMessage size must be 68 bytes");

struct OUCHReplacedMessage
{
    static constexpr OuchMessageType MSG_TYPE = OuchMessageType::REPLACED;
    uint16_t message_length;      // OUCH Message Length (excluding this field) [expect 80]
    char     packet_type;         // OUCH Packet Type  - Always ‘S’ indicating Sequenced
    char     message_type;        // OUCH Message Type - Always ‘U’ indicating Replaced
    uint64_t timestamp;
    char     order_token[14];
    char     side;
    uint32_t shares;
    char     symbol[8];
    uint32_t price;               // Price (x 10,000)
    uint32_t time_in_force;
    char     firm[4];
    char     display;
    uint64_t order_reference_number;
    char     capacity;
    char     intermarket_sweep;
    uint32_t minimum_quantity;
    char     cross_type;
    char     order_state;
    char     previous_order_token[14];

    uint32_t getShares()      const { return ntohl(shares); }
    char     getMessageType() const { return message_type;  }
};
static_assert(sizeof(OUCHReplacedMessage) == 82, "OUCHReplacedMessage size must be 82 bytes");

struct OUCHExecutedMessage
{
    static constexpr OuchMessageType MSG_TYPE = OuchMessageType::EXECUTED;

    uint16_t message_length;    // OUCH Message Length (excluding this field) - expect 41
    char     packet_type;       // OUCH Packet Type  - Always 'S' indicating Sequenced
    char     message_type;      // OUCH Message Type - Always 'E' indicating Executed
    uint64_t timestamp;
    char     order_token[14];
    uint32_t executed_shares;
    uint32_t executed_price;    // Executed Price (x 10,000)
    char     liquidity_flag;
    uint64_t match_number;

    uint32_t getShares() const { return ntohl(executed_shares); }
    char getMessageType() const { return message_type; }
};
static_assert(sizeof(OUCHExecutedMessage) == 43, "OUCHExecutedMessage size must be 43 bytes");

struct OUCHCanceledMessage {
    static constexpr OuchMessageType MSG_TYPE = OuchMessageType::CANCELED;

    uint16_t message_length;    // OUCH Message Length (excluding this field) - expect 29
    char     packet_type;       // OUCH Packet Type  - Always 'S' indicating Sequenced
    char     message_type;      // OUCH Message Type - Always 'C' indicating Canceled
    uint64_t timestamp;
    char     order_token[14];
    uint32_t decrement_shares;
    char     reason;

    uint32_t getShares() const { return ntohl(decrement_shares); }
    char getMessageType() const { return message_type; }
};
static_assert(sizeof(OUCHCanceledMessage) == 31, "OUCHCanceledMessage size is not 31 bytes");

#pragma pack(pop)  // Restore original packing


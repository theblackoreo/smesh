#ifndef ACK_TAG_H
#define ACK_TAG_H

#include "ns3/tag.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include <iostream>

using namespace ns3;

class AckTag : public Tag {
public:
  AckTag();
  void SetAckId(uint32_t id);
  uint32_t GetAckId() const;

  // Overriding serialization methods
  virtual void Serialize(TagBuffer i) const;
  virtual void Deserialize(TagBuffer i);
  virtual uint32_t GetSerializedSize() const;
  virtual void Print(std::ostream &os) const;

  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  bool IsAck();

private:
  uint32_t m_ackId; // Stores the ACK ID
  bool m_isAck = true;
};

#endif // ACK_TAG_H

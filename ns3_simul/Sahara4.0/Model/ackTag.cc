#include "ackTag.h"

NS_LOG_COMPONENT_DEFINE("AckTag");

AckTag::AckTag() : m_ackId(0) {}

void AckTag::SetAckId(uint32_t id) {
  m_ackId = id;
}

uint32_t AckTag::GetAckId() const {
  return m_ackId;
}

void AckTag::Serialize(TagBuffer i) const {
  i.WriteU32(m_ackId);
}

void AckTag::Deserialize(TagBuffer i) {
  m_ackId = i.ReadU32();
}

uint32_t AckTag::GetSerializedSize() const {
  return 4; // 32-bit unsigned int
}

void AckTag::Print(std::ostream &os) const {
  os << "ACK ID=" << m_ackId;
}

TypeId AckTag::GetTypeId(void) {
  static TypeId tid = TypeId("AckTag")
    .SetParent<Tag>()
    .AddConstructor<AckTag>();
  return tid;
}

TypeId AckTag::GetInstanceTypeId(void) const {
  return GetTypeId();
}

bool AckTag::IsAck(){
    return m_isAck;
}
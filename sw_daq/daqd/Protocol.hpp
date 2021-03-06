#ifndef __PROTOCOL_HPP__DEFINED__
#define __PROTOCOL_HPP__DEFINED__

#include <stdint.h>

namespace DAQd {

static const uint16_t commandAcqOnOff = 0x01;
static const uint16_t commandGetDataFrameSharedMemoryName = 0x02;
static const uint16_t commandGetDataFrameWriteReadPointer = 0x03;
static const uint16_t commandSetDataFrameReadPointer = 0x04;
static const uint16_t commandToFrontEnd = 0x05;
static const uint16_t commandGetPortUp = 0x06;
static const uint16_t commandGetPortCounts = 0x07;

}
#endif

/*
 * Copyright (C) 2022 GeoSn0w (@FCE365)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef KERNEL_H
#define KERNEL_H
#include "kerneltypes.h"
typedef unsigned char                uint8_t;
typedef unsigned short int        uint16_t;

kern_return_t initializeFrameBuffer(void);
kern_return_t micrOS_vanityPrint(void);
kern_return_t initializeCoreStorage(void);
kern_return_t initializeHardwareRandomNumberGenerator(void);
kern_return_t kRebootDevice(void);
kern_return_t kShutdownDevice(void);
kern_return_t corePowerManagement(corePowerManagement_cmd powerAction);
kern_return_t powerOnSelfTest(void);
unsigned long getCurrentExecutionLevel(void);
kern_return_t bootImageAtAddress(uint8_t address, int bootFlag);

#endif

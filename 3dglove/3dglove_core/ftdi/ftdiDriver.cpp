
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _FTDIDRV_IMPL
#define _FTDIDRV_IMPL

// this file interfaces to TLC5947 via FTDI

#include "ftdiDriver.h"

ftdiDriver::ftdiDriver() :
    update_cnt(0),
    ftStatus(FT_OK),
    ftHandle(nil),
    portNumber(-1),
    dwNumDevs(0),
    dwNumBytesToRead(0),
    dwNumBytesRead(0),
    dwCount(0),
    dwNumBytesToSend(0),
    dwNumBytesSent(0),
    dwClockDivisor(29)  //Value of clock divisor, SCL Frequency = 60/((1+29)*2) (MHz) = 1Mhz
    //  TODO: didn't work -> 2; //Value of clock divisor, SCL Frequency = 60/((1+2)*2) (MHz) = 10Mhz
{
}

//--------------------------
ftdiDriver::~ftdiDriver()
{
}

//--------------------------
bool ftdiDriver::start(string sn) {
    
    if ( ! init_ftdi(sn,&portNumber))
    {
        fprintf(stderr,"\n\nFTDI INIT FAIL [%s]\n",sn.c_str());
        fprintf(stderr,"MacOS: unload Mac driver: 'sudo kextunload -bundle-id com.apple.driver.AppleUSBFTDI'\n\n");
        return false;
    }
    
    ftdi_sn = sn;
    dbprintf("Found FTDI %s at port %d\n",ftdi_sn.c_str(),portNumber);
    
    if ( ! ftdi_establish_sync())
    {
        fprintf(stderr,"FTDI SYNC FAIL [%s]\n",ftdi_sn.c_str());
        return false;
    }
    
    return true;
}

//--------------------------
void ftdiDriver::stop() {
    
    dbprintf("FTDI[%d]: stopping\n",portNumber);
    
    // stop ftdi
    (void)ftdi_closeDown(true);
}

//--------------------------
void ftdiDriver::make_collision_updates(const HandVector::hand_vecs& hand)
{
    for (int f=0; f<MAX_FINGERS; ++f) {
        for (int j=0; j<MAX_JOINTS; ++j) {
            
            // is_this_special_thumb_joint(f,j)
            if ( ! ((f==0) && (j==1)) ) {
            
                float cf = hand.fingers[f].joints[j].collision_force;
                if (IS_NOT_FLOAT_ZERO(cf)) { // cf > 0.0f) {
                    // TODO: float ofNormalize(float value, float min, float max) vs. ofMap() ??
                    float gs = ::min((float)GS_MAX,cf/MAX_IMPULSE_DATA*(float)GS_MAX);
                    
                    dbprintf("MCU[%d:%d]:(%f)%d->%d\n",f,j,cf,hand.fingers[f].joints[j].ftdi_idx,(int)gs);
                    update(hand.fingers[f].joints[j].ftdi_idx,(int)gs);
                }
            }
        }
    }
}

//--------------------------
void ftdiDriver::update(int location, int force) {
    
    assert( location < GS_CHANNELS );
    
    force = min(force,GS_MAX);
    
    //dbprintf("GS[%s]:%d -> 0x%X, ",ftdi_sn.c_str(),location,force);

    int byte0 = (((GS_CHANNELS-1) - location) * 3) / 2;
    int byte1 = byte0 + 1;
    int force0, force1;
    
    update_cnt = 2;
    
    if (location & 0x1)
    {
        // odd
        //    11 10 9 8 7 6 5 4 3 2 1 0
        //   |      byte0      |  byte1 X X X X |
        force0 = (force >> 4) & 0xFF;
        force1 = (force & 0x0F) << 4;
        
        gs_data[byte0] = (BYTE)force0;
        gs_data[byte1] &= 0x0F;
        gs_data[byte1] |= (BYTE)force1;
        
        //dbprintf("GSodd[%d:0x%X] [%d]=0x%X [%d]=0x%X\n",force,force,byte0,force0,byte1,force1);
    }
    else
    {
        // even
        //             11 10 9 8 7 6 5 4 3 2 1 0
        //   | X X X X byte0    |      byte1    |
        force0 = (force >> 8) & 0x0F;
        force1 = (force & 0xFF);
        
        gs_data[byte0] &= 0xF0;
        gs_data[byte0] |= (BYTE)force0;
        gs_data[byte1] = (BYTE)force1;
        
        //dbprintf("GSeven[%d:0x%X] [%d]=0x%X [%d]=0x%X\n",force,force,byte0,force0,byte1,force1);
    }
}

//--------------------------
void ftdiDriver::draw() {

    if (update_cnt > 0)
    {   
        // write 288bits to TLC5947, then clear for next write
        if( ! send_data_buffer(gs_data,GS_BYTES) )
        {
            //assert(0);
            fprintf(stderr,"FTDI: ASSERT: closing down...\n");
            stop();
        }
        ::memset(gs_data,0,sizeof(gs_data));
        
        --update_cnt;
    }
}


//////////////////
//// FTDI
//////////////////

//--------------------------
bool ftdiDriver::init_ftdi(string sn, int* port)
{
    // create the device information list
    dbprintf("Checking for FTDI devices... (SN=%s)\n",sn.c_str());
    if (FT_CreateDeviceInfoList(&dwNumDevs) != FT_OK)
    {
        fprintf(stderr,"FTDI[%s]: FT_CreateDeviceInfoList() failed\n",sn.c_str());
        return false;
    }
    if (dwNumDevs <= 0)
    {
        fprintf(stderr,"No devices found!\n");
        return false;
    }
    dbprintf("Found %d devices in device list\n",dwNumDevs);
    
    // look for SN and get port
    char *  pcBufLD[FTDI_MAX_DEVICES + 2];  // TODO: +2 ??
    char    cBufLD[FTDI_MAX_DEVICES][64];
    
    for(int i = 0; i < FTDI_MAX_DEVICES; ++i)
        pcBufLD[i] = cBufLD[i];
    pcBufLD[FTDI_MAX_DEVICES+1] = NULL;     // TODO: +1 ??
    
    ftStatus = FT_ListDevices(pcBufLD, &dwNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
    
    dbprintf("Found %d devices by SN\n",dwNumDevs);
    
    if(ftStatus != FT_OK) {
        printf("Error: FT_ListDevices(%d)\n", (int)ftStatus);
        return 1;
    }

    portNumber = FTDI_MAX_DEVICES+1;
    
    for(int i = 0; ( (i <FTDI_MAX_DEVICES) && (i < dwNumDevs) ); ++i) {
        dbprintf("Device %d Serial Number - %s\n", i, cBufLD[i]);
        if ( ! strcmp( cBufLD[i], sn.c_str() ) )
        {
            portNumber = i;
            break;
        }
    }
    
    if (portNumber == FTDI_MAX_DEVICES+1)
    {
        fprintf(stderr,"Could not find FTDI with SN=%s\n",sn.c_str());
        return 1;
    }
    
    assert(port);
    *port = portNumber;
    
	ftStatus |= FT_Open(portNumber, &ftHandle);
	if (ftStatus != FT_OK)
	{
		/* FT_Open can fail if the ftdi_sio module is already loaded. */
		printf("FT_Open(%d:%s) failed (error %d).\n",portNumber,sn.c_str(),(int)ftStatus);
		printf("Use lsmod to check if ftdi_sio (and usbserial) are present.\n");
		printf("If so, unload them using rmmod, as they conflict with ftd2xx.\n");
		return false;
	}
    
    dbprintf("Configuring port for MPSSE use...\n");
    
    // reset device
    ftStatus |= FT_ResetDevice(ftHandle);
    ftStatus |= FT_SetTimeouts(ftHandle,10,0);
    
    // purge USB receive buffer first by reading out all old data from FT2232H receive buffer
    ftStatus |= FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);
    
    // get the number of bytes in the FT2232H receive buffer
    if ((ftStatus == FT_OK) && (dwNumBytesToRead > 0))
        FT_Read(ftHandle, &byInputBuffer, dwNumBytesToRead, &dwNumBytesRead);
    
    // 10ms timeout for both read and write to device
    ftStatus |= FT_SetTimeouts(ftHandle, 100, 100);
    // 64 bytes xfer each way
    ftStatus |= FT_SetUSBParameters(ftHandle, BYTES_TO_TLC5947,BYTES_TO_TLC5947);
    // disable events & error chars
    ftStatus |= FT_SetChars(ftHandle,false,0,false,0);
    // set latency for faster response, 2ms is min
    ftStatus |= FT_SetLatencyTimer(ftHandle,2);
    // turn on flow control to synchronize IN requests
    ftStatus |= FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0x00, 0x00);
    // reset MPSSE itself
    ftStatus |= FT_SetBitMode(ftHandle,0,0);
    // enable MPSSE
    ftStatus |= FT_SetBitMode(ftHandle,0,2);
    // let settle
    usleep(50000);
    if (ftStatus != FT_OK)
    {
        fprintf(stderr,"Error:init_ftdi[%d:%s] initializing MPSSE %d\n",portNumber,ftdi_sn.c_str(),ftStatus);
        return ftdi_closeDown(false);
    }

    dwNumBytesToSend = 0;
    // Disables the clk divide by 5 to allow for a 60MHz master clock.
	byOutputBuffer[dwNumBytesToSend++] = 0x8A;
	// Disable adaptive clocking
	byOutputBuffer[dwNumBytesToSend++] = 0x97;
	// disable 3 phase data clock
	byOutputBuffer[dwNumBytesToSend++] = 0x8D;
	ftStatus |= FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    
	/*  Connections to TLC5947: (pins here are at CN2 connector, not on FT2232H IC)
     ADBUS0 TCK/SK ---> CLK (pin2 orange SK)
     ADBUS1 TDI/DO -+-> DIN (pin3 yellow DO)
     ADBUS2 TDO/DI -+   n/c
     ADBUS3 TMS/CS --> LATCH (pin5 brown CS)
     ADBUSS GPIOL0 --> BLANK (pin6 grey GPIOL0)
     ADBUS5 GPIOL1
     ADBUS6 GPIOl2
     ADBUS7 GPIOL3
     */
    
    // configure pins init state
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0x80;  // set output ACBUS 0-7
    byOutputBuffer[dwNumBytesToSend++] = 0x10;  // value
    byOutputBuffer[dwNumBytesToSend++] = 0x1b;  // direction
    ftStatus |= FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    // set clock divisor
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0x86;
    byOutputBuffer[dwNumBytesToSend++] = (dwClockDivisor & 0xff);
    byOutputBuffer[dwNumBytesToSend++] = (dwClockDivisor >> 8) & 0xff;
    ftStatus |= FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    
    // Turn of Loopback
    byOutputBuffer[0] = 0x85;
    ftStatus |= FT_Write(ftHandle, byOutputBuffer, 1, &dwNumBytesSent);
    if (ftStatus != FT_OK)
    {
        fprintf(stderr,"FDTI:init_ftdi[%d:%s] Error initializing MPSSE #2 %d\n",portNumber,ftdi_sn.c_str(),ftStatus);
        return ftdi_closeDown(false);
    }

    // want to send GS data to TLC5947 then lower #OE
    // always want #OE low, then we can clock high/low after sending data
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0x11;  // Clock Data Bytes Out on -ve clock edge MSB first (no read)
    byOutputBuffer[dwNumBytesToSend++] = GS_BYTES-1;  // LengthL
    byOutputBuffer[dwNumBytesToSend++] = 0x00;  // LengthH
    for (int ix=0; ix < GS_BYTES; ++ix)
        byOutputBuffer[dwNumBytesToSend++] = 0x00;
    byOutputBuffer[dwNumBytesToSend++] = 0x80;  // set output ACBUS 0-7
    byOutputBuffer[dwNumBytesToSend++] = 0x00;  // value
    byOutputBuffer[dwNumBytesToSend++] = 0x1b;  // direction
    byOutputBuffer[dwNumBytesToSend++] = 0x80;  // set output ACBUS 0-7
    byOutputBuffer[dwNumBytesToSend++] = 0x08;  // value
    byOutputBuffer[dwNumBytesToSend++] = 0x1b;  // direction
    byOutputBuffer[dwNumBytesToSend++] = 0x80;  // set output ACBUS 0-7
    byOutputBuffer[dwNumBytesToSend++] = 0x00;  // value
    byOutputBuffer[dwNumBytesToSend++] = 0x1b;  // direction
    ftStatus |= FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    if (ftStatus != FT_OK)
    {
        fprintf(stderr,"FTDI:init_ftdi[%d:%s] Error clearing all data on TLC5947 %d\n",portNumber,ftdi_sn.c_str(),ftStatus);
        return ftdi_closeDown(false);
    }
    
    // success
    return true;
}

//--------------------------
bool ftdiDriver::ftdi_establish_sync()
{
    // make sure we have sync w/ MPSSE
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0xAA; //Add BAD command
    ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent); // Send off the BAD commands
    dwNumBytesToSend = 0; //Clear output buffer
    do{
        ftStatus = FT_GetQueueStatus(ftHandle, &dwNumBytesRead); // Get the number of bytes in the device input buffer
    }while ((dwNumBytesRead == 0) && (ftStatus == FT_OK)); //or Timeout
    
    bool bCommandEchod = false;
    ftStatus = FT_Read(ftHandle, byInputBuffer, dwNumBytesRead, &dwNumBytesToRead); //Read out the data from input buffer
    for (dwCount = 0; dwCount < (dwNumBytesToRead - 1); dwCount++) //Check if Bad command and echo command received
    {
        if ((byInputBuffer[dwCount] == 0xFA) && (byInputBuffer[dwCount+1] == 0xAA))
        {
            bCommandEchod = true;
            break;
        }
    }
    if (bCommandEchod == false)
    {
        printf("ERROR:ftdi_establish_sync[%d:%s] fail to synchronize MPSSE with command '0xAA' \n",portNumber,ftdi_sn.c_str());
        return ftdi_closeDown(false);
    }

    // success
    return true;
}

//--------------------------
bool ftdiDriver::set_red_LED(bool turn_on)
{
    // set state on red LED on FTDI plug
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0x82;  // set output ACBUS 0-7
    byOutputBuffer[dwNumBytesToSend++] = (turn_on) ? 0x00 : 0x40;  // value
    byOutputBuffer[dwNumBytesToSend++] = 0x40;  // direction
    ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    if (ftStatus != FT_OK)
    {
        fprintf(stderr,"Error:set_red_LED[%d:%s] initializing MPSSE %d (loopback)\n",portNumber,ftdi_sn.c_str(),ftStatus);
        return ftdi_closeDown(false);
    }
    
    // success
    return true;
}

//--------------------------
bool ftdiDriver::send_data_buffer(BYTE * buf, DWORD cnt)
{
    assert(cnt <= GS_BYTES);
    
    // fill SPI data buffer for TLC5947
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0x11;  // Clock Data Bytes Out on -ve clock edge MSB first (no read)
    byOutputBuffer[dwNumBytesToSend++] = cnt-1;  // LengthL
    byOutputBuffer[dwNumBytesToSend++] = 0x00;  // LengthH
    for (int ix=0; ix < cnt; ++ix)
        byOutputBuffer[dwNumBytesToSend++] = *buf++;
    // toggle LATCH (CS) to activate change
    byOutputBuffer[dwNumBytesToSend++] = 0x80;  // set output ACBUS 0-7
    byOutputBuffer[dwNumBytesToSend++] = 0x08;  // value
    byOutputBuffer[dwNumBytesToSend++] = 0x1b;  // direction
    byOutputBuffer[dwNumBytesToSend++] = 0x80;  // set output ACBUS 0-7
    byOutputBuffer[dwNumBytesToSend++] = 0x00;  // value
    byOutputBuffer[dwNumBytesToSend++] = 0x1b;  // direction
    ftStatus |= FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    if (ftStatus != FT_OK)
    {
        fprintf(stderr,"Error:send_data_buffer[%d:%s] writing SPI data %d\n",portNumber,ftdi_sn.c_str(),ftStatus);
        return ftdi_closeDown(false);
    }
    
    // Check the receive buffer - it should be empty
    ftStatus |= FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);
    ftStatus |= FT_Read(ftHandle, &byInputBuffer, dwNumBytesToRead, &dwNumBytesRead);
    if (dwNumBytesToRead != 0)
    {
        printf("Error:send_data_buffer[%d:%s] - MPSSE receive buffer should be empty %d\n",portNumber,ftdi_sn.c_str(),ftStatus);
        return ftdi_closeDown(false);
    }
    if (ftStatus != FT_OK)
    {
        fprintf(stderr,"Error:send_data_buffer[%d:%s] read zero SPI data %d\n",portNumber,ftdi_sn.c_str(),ftStatus);
        return ftdi_closeDown(false);
    }
    
    // success
    return true;
}

//--------------------------
bool ftdiDriver::ftdi_closeDown(bool ret)
{
    fprintf(stderr,"FTDI: closing down [%d:%s]\n",portNumber,ftdi_sn.c_str());
	(void)FT_SetBitMode(ftHandle,
	                    0, /* ignored with FT_BITMODE_RESET */
	                    FT_BITMODE_RESET);
    
	(void)FT_Close(ftHandle);
    return ret;
}

#endif

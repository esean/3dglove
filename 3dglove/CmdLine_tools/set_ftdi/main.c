/*
 * Assuming libftd2xx.dylib is in /usr/local/lib, build with:
 * 
 *     gcc -o mpsse_demo main.c -L. -lftd2xx -Wl,-rpath /usr/local/lib -L/usr/local/lib
 * 
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "../../3dglove_core/ftdi/ftd2xx.h"
#include <assert.h>
#include <string.h>
#include "stdlib.h"

#define FTDI_MAX_DEVICES 2

#define BYTES_TO_TLC5947 64 // usb data

#define GS_MAX              ((1<<12)-1) // 4095 // 2^12
#define GS_CHANNELS         24
#define GS_BYTES            ((GS_CHANNELS*12)/8)  // bytes, ie 288bits/8 -> TLC5947

static BYTE byOutputBuffer[BYTES_TO_TLC5947];
static BYTE tlc_buf[GS_BYTES];

//--------------------------
void update(int location, int force) {
    
    assert( location < GS_CHANNELS );
    
    //force = min(force,GS_MAX);
    if (force > GS_MAX)
        force = GS_MAX;
    
    //dbprintf("GS[%s]:%d -> 0x%X, ",ftdi_sn.c_str(),location,force);
    
    int byte0 = (((GS_CHANNELS-1) - location) * 3) / 2;
    int byte1 = byte0 + 1;
    int force0, force1;
    
    if (location & 0x1)
    {
        // odd
        //    11 10 9 8 7 6 5 4 3 2 1 0
        //   |      byte0      |  byte1 X X X X |
        force0 = (force >> 4) & 0xFF;
        force1 = (force & 0x0F) << 4;
        
        tlc_buf[byte0] = (BYTE)force0;
        tlc_buf[byte1] &= 0xF0;
        tlc_buf[byte1] |= (BYTE)force1;
        
        //dbprintf("GSodd[%d:0x%X] [%d]=0x%X [%d]=0x%X\n",force,force,byte0,force0,byte1,force1);
    }
    else
    {
        // even
        //             11 10 9 8 7 6 5 4 3 2 1 0
        //   | X X X X byte0    |      byte1    |
        force0 = (force >> 8) & 0x0F;
        force1 = (force & 0xFF);
        
        tlc_buf[byte0] &= 0x0F;
        tlc_buf[byte0] |= (BYTE)force0;
        tlc_buf[byte1] = (BYTE)force1;
        
        //dbprintf("GSeven[%d:0x%X] [%d]=0x%X [%d]=0x%X\n",force,force,byte0,force0,byte1,force1);
    }
}

int main(int argc, char **argv)
{
	FT_STATUS	ftStatus = FT_OK;
	FT_HANDLE	ftHandle;
	int         portNumber = 0;
    DWORD dwNumDevs;
    BYTE byInputBuffer[BYTES_TO_TLC5947];
    DWORD dwCount = 0;
    DWORD dwNumBytesToSend = 0;
    DWORD dwNumBytesSent = 0;
    DWORD dwNumBytesToRead = 0;
    DWORD dwNumBytesRead = 0;
    DWORD dwClockDivisor = 29; //Value of clock divisor, SCL Frequency = 60/((1+29)*2) (MHz) = 1Mhz
    //TBD: didn't work... DWORD dwClockDivisor = 2; //Value of clock divisor, SCL Frequency = 60/((1+2)*2) (MHz) = 10Mhz
    
    if (argc <= 1)
    {
        printf("\nUSAGE: [app] [[idx][intensity]] ...\n");
        exit(-1);
    }
    
    (void)argc;
    (void)argv;
    
    // create the device information list
    printf("Checking for FTDI devices...\n");
    if (FT_CreateDeviceInfoList(&dwNumDevs) != FT_OK)
    {
        fprintf(stderr,"FT_CreateDeviceInfoList() failed\n");
        return(-1);
    }
    if (dwNumDevs <= 0)
    {
        fprintf(stderr,"No devices found!\n");
        return (-1);
    }
    printf("Found %d devices\n",dwNumDevs);

    // look for SN and get port
    char *  pcBufLD[FTDI_MAX_DEVICES + 2];  // TODO: +2 ??
    char    cBufLD[FTDI_MAX_DEVICES][64];
    
    for(DWORD i = 0; i < FTDI_MAX_DEVICES; ++i)
        pcBufLD[i] = cBufLD[i];
    pcBufLD[FTDI_MAX_DEVICES+1] = NULL;     // TODO: +1 ??
    
    ftStatus = FT_ListDevices(pcBufLD, &dwNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
    
    printf("Found %d devices by SN\n",dwNumDevs);
    
    if(ftStatus != FT_OK) {
        printf("Error: FT_ListDevices(%d)\n", (int)ftStatus);
        return 1;
    }

    for(DWORD i = 0; ( (i <FTDI_MAX_DEVICES) && (i < dwNumDevs) ); ++i) {
        printf("Device %d Serial Number - %s\n", i, cBufLD[i]);
    }

    // TODO: assume the one found has MPSSE and use it

#if 0
    // my FT2232H has this vid/pid
    if (FT_SetVIDPID(0x403,0x6017) != FT_OK)
    {
        printf("FT_SetVIDPID() failed\n");
        return(-1);
    }
#endif
    
	ftStatus |= FT_Open(portNumber, &ftHandle);
	if (ftStatus != FT_OK) 
	{
		/* FT_Open can fail if the ftdi_sio module is already loaded. */
		printf("FT_Open(%d) failed (error %d).\n", portNumber, (int)ftStatus);
		printf("Use lsmod to check if ftdi_sio (and usbserial) are present.\n");
		printf("If so, unload them using rmmod, as they conflict with ftd2xx.\n");
        fprintf(stderr,"\n\nFTDI INIT FAIL\n");
        fprintf(stderr,"MacOS: unload Mac driver: 'sudo kextunload -bundle-id com.apple.driver.AppleUSBFTDI'\n\n");
		return 1;
	}
    
    printf("Configuring port for MPSSE use...\n");

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
    ftStatus |= FT_SetUSBParameters(ftHandle, 64,64);
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
        fprintf(stderr,"Error initializing MPSSE %d\n",ftStatus);
        goto exit;
    }

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
        printf("fail to synchronize MPSSE with command '0xAA' \n");
        goto exit;
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
        fprintf(stderr,"Error initializing MPSSE #2 %d\n",ftStatus);
        goto exit;
    }
    

    ///////////////////////
    //// READY TO GO !
    ///////////////////////
    
#if 0
    // turn on red LED on FTDI plug
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0x82;  // set output ACBUS 0-7
    byOutputBuffer[dwNumBytesToSend++] = 0x00;  // value
    byOutputBuffer[dwNumBytesToSend++] = 0x40;  // direction
    ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    if (ftStatus != FT_OK)
    {
        fprintf(stderr,"Error initializing MPSSE %d (loopback)\n",ftStatus);
        goto exit;
    }
    
    // want to send GS data to TLC5947 then lower #OE
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
        fprintf(stderr,"Error writing SPI data %d\n",ftStatus);
        goto exit;
    }
    printf("SPI write %d bytes\n",dwNumBytesSent);
#endif
    
    // lower #OE
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0x80;  // set output ADBUS 0-7
    byOutputBuffer[dwNumBytesToSend++] = 0x00;  // value
    byOutputBuffer[dwNumBytesToSend++] = 0x1b;  // direction
    ftStatus |= FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    
    // now write out any values we want
    /////////////////////
    memset(byOutputBuffer,0,sizeof(byOutputBuffer));
    memset(tlc_buf,0,sizeof(tlc_buf));
    
    // fill in desired idx/level from user
    int ii = 1;
    while (ii < argc)
    {
        printf("# Set idx:%d => intensity:%d\n",atoi(argv[ii]),atoi(argv[ii+1]));
        update(atoi(argv[ii]),atoi(argv[ii+1]));
        ii += 2;
    }
    
    // fill SPI data buffer for TLC5947
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0x11;  // Clock Data Bytes Out on -ve clock edge MSB first (no read)
    byOutputBuffer[dwNumBytesToSend++] = GS_BYTES-1;  // LengthL
    byOutputBuffer[dwNumBytesToSend++] = 0x00;  // LengthH
    
    // bring in cfg data
    for (int ix=0; ix < GS_BYTES; ++ix)
        byOutputBuffer[dwNumBytesToSend++] = tlc_buf[ix];

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
        fprintf(stderr,"Error writing SPI data RET=%d\n",ftStatus);
        goto exit;
    }
    
    // Check the receive buffer - it should be empty
    ftStatus |= FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);
    ftStatus |= FT_Read(ftHandle, &byInputBuffer, dwNumBytesToRead, &dwNumBytesRead);
    if (dwNumBytesToRead != 0)
    {
        printf("Error - MPSSE receive buffer should be empty RET=%d\n", ftStatus);
        goto exit;
    }
    if (ftStatus != FT_OK)
    {
        fprintf(stderr,"Error read zero SPI data RET=%d\n",ftStatus);
        goto exit;
    }
    
    // leave it on for second
    sleep(3);
    
    // success
    printf("Done, reseting MPSSE before exiting...\n");
    goto exitfg;
exit:
    fprintf(stderr,"ERROR: MPSSE is being reset before exiting !\n");
exitfg:
	/* Return chip to default (UART) mode. */
	(void)FT_SetBitMode(ftHandle, 
	                    0, /* ignored with FT_BITMODE_RESET */
	                    FT_BITMODE_RESET);

	(void)FT_Close(ftHandle);
	return 0;
}

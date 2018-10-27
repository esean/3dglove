
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _FTDIDRV_OBJECT
#define _FTDIDRV_OBJECT

#include "3dtarget.h"
#include "ftd2xx.h"

class ftdiDriver {

	public:
    
		//--------------------------
        ftdiDriver();
        virtual ~ftdiDriver();
    
        bool start(string sn);
        void stop();
    
        bool set_red_LED(bool turn_on);

        void make_collision_updates(const HandVector::hand_vecs& hand);
    
        void update(int location, int force);
        void draw();
    
private:
    
    // tlc5947
    BYTE gs_data[GS_BYTES];
    
    // ftdi
    FT_STATUS	ftStatus;
    FT_HANDLE	ftHandle;
    int         portNumber;
    string      ftdi_sn;
    // TODO: do we need all these here, or can be local?
    DWORD dwNumDevs;
    BYTE byOutputBuffer[BYTES_TO_TLC5947];
    BYTE byInputBuffer[BYTES_TO_TLC5947];
    DWORD dwCount;
    DWORD dwNumBytesToSend;
    DWORD dwNumBytesSent;
    DWORD dwNumBytesToRead;
    DWORD dwNumBytesRead;
    DWORD dwClockDivisor;
    bool init_ftdi(string sn, int* port);
    bool ftdi_establish_sync();
    bool ftdi_closeDown(bool ret);
    bool send_data_buffer(BYTE * buf, DWORD cnt);

    int update_cnt;
};

#endif  // _FTDIDRV_OBJECT


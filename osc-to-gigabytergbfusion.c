// To compile
//   tcc osc-to-gigabytergbfusion.c -run -lsetupapi -lws2_32

// For Gigabyte Aorus motherboards.
// really cool that you can smoothly control colors on this controller!

#include <stdio.h>
#include <windows.h>
#include "hidapi.h"
#include "hidapi.c"

#define MINIOSC_IMPLEMENTATION
#include "miniosc.h"

// I tried reading this and couldn't figure out how to use it:
// https://github.com/CalcProgrammer1/OpenRGB/blob/dfcc4364721f4b2057ea6a8ffe8a85c014019d52/Controllers/GigabyteRGBFusion2USBController/RGBController_GigabyteRGBFusion2USB.cpp
// https://github.com/CalcProgrammer1/OpenRGB/blob/dfcc4364721f4b2057ea6a8ffe8a85c014019d52/Controllers/GigabyteRGBFusion2USBController/GigabyteRGBFusion2USBController.h
// Instead I just used wireshark and copied the commands.

hid_device * dev;

void Update( uint32_t * colorval, int all )
{
	uint8_t databuff[64];
	uint8_t * dptr = databuff;
	
	int cvled = 0;
	
	// LEDs on header
	int group = 0;
	int led = 0;
	for( group = 0; group < 4; group++ )
	{
		dptr = databuff;
		*(dptr++) = 0xcc;
		*(dptr++) = 0x58;
		*(dptr++) = group * 0x39;
		*(dptr++) = 0x00;
		*(dptr++) = 0x39;
	
		int i = 0;
		for( i = 0; i < 19; i++ )
		{
			*(dptr++) = (colorval[cvled]>>8)&0xff;
			*(dptr++) = (colorval[cvled]>>0)&0xff;
			*(dptr++) = (colorval[cvled]>>16)&0xff;
			if( !all ) cvled++;
		}
		*(dptr++) = 0;
		*(dptr++) = 0;
		int r = hid_send_feature_report( dev, databuff, 64 );
	}

	// Mobo LEDs
	int mark = 0;
	for( mark = 0; mark < 5; mark++ )
	{
		int bv = 1<<mark;
		dptr = databuff;
		*(dptr++) = 0xcc;
		*(dptr++) = 0x20+mark;
		*(dptr++) = bv;
		*(dptr++) = 0x00;

		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x01;
		
		*(dptr++) = 0x64;
		*(dptr++) = 0x00;
		*(dptr++) = (colorval[cvled]>>16)&0xff;
		*(dptr++) = (colorval[cvled]>>8)&0xff;
		*(dptr++) = (colorval[cvled]>>0)&0xff;
		if( !all ) cvled++;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;

		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0xb0;
		*(dptr++) = 0x04;
		*(dptr++) = 0xc8;
		*(dptr++) = 0x00;
		
		*(dptr++) = 0xc8;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x00;
		*(dptr++) = 0x01;
		int r = hid_send_feature_report( dev, databuff, 64 );
	}
	memset( databuff, 0, sizeof( databuff ) );
	
	dptr = databuff;
	*(dptr++) = 0xcc;
	*(dptr++) = 0x28;
	*(dptr++) = 0xff;
	*(dptr++) = 0x00;
	int r = hid_send_feature_report( dev, databuff, 64 );
}

void oscCallback( const char * address, const char * type, void ** parameters )
{
	if( strcmp( address, "/opc/zone6" ) == 0 )
	{
		if( strcmp( type, ",r" ) == 0 )
		{
			uint32_t colorval = *((uint32_t*)parameters[0]);
			Update( &colorval, 1 );
		}
		else
		{
			printf( "Unknown parmaeters: %s\n", type );
		}
	}
	if( strcmp( address, "/opc/zall" ) == 0 )
	{
		Update( (uint32_t*)parameters, 0 );
	}
	else
	{
		printf( "Unknown message: %s\n", address );
	}
}

int main()
{
	hid_init();
	dev = hid_open_path(  "\\\\?\\hid#vid_048d&pid_8297&col02#9&3868a9be&0&0001#{4d1e55b2-f16f-11cf-88cb-001111000030}" ); //hid_open( 0x048d, 0x8297, 0 );
	printf( "Device: %p\n", dev );
	if( !dev )
	{
		fprintf( stderr, "Error: No Polychrome USB Motherboard detected.\n" );
		return -5;
	}
	
	int minioscerrorcode = 0;
	miniosc * osc = minioscInit( 9993, 0, 0, &minioscerrorcode );
	if( !osc )
	{
		fprintf( stderr, "Error: could not initialize miniosc. Error code: %d\n", minioscerrorcode );
		return -6;
	}
	
	int frame=  0;
	while(1)
	{
		minioscPoll( osc, 10, oscCallback );
	}
	
}

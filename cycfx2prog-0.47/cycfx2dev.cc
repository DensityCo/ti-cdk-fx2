/*
 * cycfx2dev.cc - Cypress FX2 device class: low-level routines. 
 * 
 * Copyright (c) 2006--2009 by Wolfgang Wieser ] wwieser (a) gmx <*> de [ 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#include "cycfx2dev.h"


struct usb_device *USBFindDevice(const char *bus,const char *dev)
{
	for(usb_bus *b=usb_busses; b; b=b->next)
	{
		if(strcmp(b->dirname,bus))  continue;
		for(struct usb_device *d=b->devices; d; d=d->next)
		{
			if(strcmp(d->filename,dev))  continue;
			return(d);
		}
	}
	return(NULL);
}

struct usb_device *USBFindDevice(int vendor,int product,int nth)
{
	for(usb_bus *b=usb_busses; b; b=b->next)
	{
		for(struct usb_device *d=b->devices; d; d=d->next)
		{
			if(d->descriptor.idVendor==vendor && 
				d->descriptor.idProduct==product)
			{
				if(!nth--)
				{  return(d);  }
			}
		}
	}
	return(NULL);
}

//------------------------------------------------------------------------------

int CypressFX2Device::BlockRead(int endpoint,unsigned char *buf,size_t nbytes,
	char type)
{
	// FIXME: This function is somewhat bugged concerning reliable delivery 
	//        and correct handling of return values for short reads and . 
	//        timeout handling. Reason: Not sure what libusb is meant to 
	//        return and to do in non-standard cases. 
	
	if(!IsOpen())
	{  fprintf(stderr,"BlockRead: Not connected!\n");  return(-1);  }
	
	bool may_be_short=0;
	switch(type)
	{
		case 'b':  break;
		case 'i':  break;
		case 'B':  may_be_short=1;  type='b';  break;
		case 'I':  may_be_short=1;  type='i';  break;
		default:  assert(0);
	}
	
	int interface=0;
	int alt_interface=(type=='i' ? 2 : 1);
	if(force_alt_interface>=0)  alt_interface=force_alt_interface;
	if(usb_claim_interface(usbhdl,interface)<0)
	{  fprintf(stderr,"Failed to claim interface %d: %s\n",
		interface,usb_strerror());  return(-1);  }
	
	size_t chunk_size=nbytes;
	int error=0;
	size_t left=nbytes;
	do {
		if(usb_set_altinterface(usbhdl,alt_interface)<0)
		{
			fprintf(stderr,"Failed to set altinterface %d: %s\n",
				alt_interface,usb_strerror());
			++error;  break;
		}
		
		int ncalls=0;
		while(left)
		{
			size_t bs = left>chunk_size ? chunk_size : left;
			ssize_t rv;
			if(type=='i')
			{  rv=usb_interrupt_read(usbhdl,endpoint,(char*)buf,bs,
				/*timeout=*/1000/*msec*/);  }
			else
			{  rv=usb_bulk_read(usbhdl,endpoint,(char*)buf,bs,
				/*timeout=*/1000/*msec*/);  }
			++ncalls;
			if(rv<0)
			{
				fprintf(stderr,"Reading %zu bytes from EP 0x%02x: "
					"USB: %s SYS: %s\n",
					bs,endpoint,usb_strerror(),strerror(-rv));
				++error;
				goto breakout;
			}
			assert((size_t)rv<=left);
			left-=rv;
			if((size_t)rv<bs)
			{
				if(may_be_short) goto breakout;
				
				// Not sure if rv=0 is a valid timeout indication...
				//if(!rv)
					fprintf(stderr,"Short read (%zd/%zu bytes)%s\n",rv,bs,
						rv==0 ? " (timeout?)" : "");
				if(rv==0)
				{  ++error;  goto breakout;  }
			}
		}
	} while(0); breakout:;
	
	usb_release_interface(usbhdl,interface);
	
	size_t read=nbytes-left;
	return(read ? read : (error ? -1 : 0));
}


int CypressFX2Device::BlockWrite(int endpoint,const unsigned char *buf,
	size_t nbytes,char type)
{
	// FIXME: This function is somewhat bugged concerning reliable delivery 
	//        and correct handling of return values for short writes and . 
	//        timeout handling. Reason: Not sure what libusb is meant to 
	//        return and to do in non-standard cases. 
	
	if(!IsOpen())
	{  fprintf(stderr,"BlockWrite: Not connected!\n");  return(-1);  }
	
	switch(type)
	{
		case 'b':  break;
		case 'i':  break;
		default:  assert(0);
	}
	
	int interface=0;
	int alt_interface=(type=='i' ? 2 : 1);
	if(force_alt_interface>=0)  alt_interface=force_alt_interface;
	if(usb_claim_interface(usbhdl,interface)<0)
	{  fprintf(stderr,"Failed to claim interface %d: %s\n",
		interface,usb_strerror());  return(-1);  }
	
	size_t chunk_size=nbytes;
	int error=0;
	size_t left=nbytes;
	do {
		if(usb_set_altinterface(usbhdl,alt_interface)<0)
		{
			fprintf(stderr,"Failed to set altinterface %d: %s\n",
				alt_interface,usb_strerror());
			++error;  break;
		}
		
		int ncalls=0;
		while(left)
		{
			size_t bs = left>chunk_size ? chunk_size : left;
			ssize_t rv;
			if(type=='i')
			{  rv=usb_interrupt_write(usbhdl,endpoint,(char*)buf,bs,
				/*timeout=*/1000/*msec*/);  }
			else
			{  rv=usb_bulk_write(usbhdl,endpoint,(char*)buf,bs,
				/*timeout=*/1000/*msec*/);  }
			++ncalls;
			if(rv<0)
			{
				fprintf(stderr,"Writing %zu bytes to EP 0x%02x: "
					"USB: %s SYS: %s\n",
					bs,endpoint,usb_strerror(),strerror(-rv));
				++error;
				goto breakout;
			}
			assert((size_t)rv<=left);
			left-=rv;
			if((size_t)rv<bs)
			{
				// Not sure if rv=0 is a valid timeout indication...
				//if(!rv)
					fprintf(stderr,"Short write (%zd/%zu bytes)%s\n",rv,bs,
						rv==0 ? " (timeout?)" : "");
				if(rv==0)
				{  ++error;  goto breakout;  }
			}
		}
	} while(0); breakout:;
	
	usb_release_interface(usbhdl,interface);
	
	size_t read=nbytes-left;
	return(read ? read : (error ? -1 : 0));
}


int CypressFX2Device::BenchBlockRead(int endpoint,size_t nbytes,
	size_t chunk_size,char type)
{
	if(!IsOpen())
	{  fprintf(stderr,"BenchBlockRead: Not connected!\n");  return(1);  }
	
	assert(type=='b' || type=='i');
	
	int interface=0;
	int alt_interface=(type=='i' ? 2 : 1);
	if(force_alt_interface>=0)  alt_interface=force_alt_interface;
	if(usb_claim_interface(usbhdl,interface)<0)
	{  fprintf(stderr,"Failed to claim interface %d: %s\n",
		interface,usb_strerror());  return(1);  }
	
	int error=0;
	char *buf=(char*)malloc(chunk_size);
	assert(buf);
	do {
		if(usb_set_altinterface(usbhdl,alt_interface)<0)
		{
			fprintf(stderr,"Failed to set altinterface %d: %s\n",
				alt_interface,usb_strerror());
			++error;  break;
		}
		
		// Start benchmark: 
		timeval start_tv,end_tv;
		gettimeofday(&start_tv,NULL);
		
		size_t left=nbytes;
		int ncalls=0;
		while(left)
		{
			size_t bs = left>chunk_size ? chunk_size : left;
			ssize_t rv;
			if(type=='i')
			{  rv=usb_interrupt_read(usbhdl,endpoint,buf,bs,
				/*timeout=*/1000/*msec*/);  }
			else
			{  rv=usb_bulk_read(usbhdl,endpoint,buf,bs,
				/*timeout=*/1000/*msec*/);  }
			++ncalls;
			if(rv<0)
			{
				fprintf(stderr,"Reading %zu bytes from EP 0x%02x: "
					"USB: %s SYS: %s\n",
					bs,endpoint,usb_strerror(),strerror(-rv));
				++error;
				goto breakout;
			}
			if((size_t)rv<bs)
			{
				// Not sure if rv=0 is a valid timeout indication...
				//if(!rv)
					fprintf(stderr,"Short read (%zd/%zu bytes)%s\n",rv,bs,
						rv==0 ? " (timeout?)" : "");
				if(rv==0)
				{  ++error;  goto breakout;  }
			}
			assert((size_t)rv<=left);
			left-=rv;
		}
		
		// End benchmark: 
		gettimeofday(&end_tv,NULL);
		
		double seconds = 
			double(end_tv.tv_sec-start_tv.tv_sec)+
			double(end_tv.tv_usec-start_tv.tv_usec)/1000000.0;
		printf("Read %zu bytes in %5d msec (chunk size %6zu): "
			"%6.3f Mb/sec (%5d calls, %6zu bytes/call)\n",
			nbytes,(int)(seconds*1000+0.5),
			chunk_size,nbytes/seconds/1024/1024,
			ncalls,nbytes/ncalls);
		
	} while(0); breakout:;
	
	if(buf)
	{  free(buf);  buf=NULL;  }
	usb_release_interface(usbhdl,interface);
	
	return(error ? -1 : 0);
}


int CypressFX2Device::FX2Reset(bool running)
{
	// Reset is accomplished by writing a 1 to address 0xE600. 
	// Start running by writing a 0 to that address. 
	const size_t reset_addr=0xe600;
	unsigned char val = running ? 0 : 1;
	
	return(WriteRAM(reset_addr,&val,1));
}


int CypressFX2Device::WriteRAM(size_t addr,const unsigned char *data,
	size_t nbytes)
{
	if(!IsOpen())
	{  fprintf(stderr,"WriteRAM: Not connected!\n");  return(1);  }
	
	int n_errors=0;
	
	const size_t chunk_size=16;
	const unsigned char *d=data;
	const unsigned char *dend=data+nbytes;
	while(d<dend)
	{
		size_t bs=dend-d;
		if(bs>chunk_size)  bs=chunk_size;
		size_t dl_addr=addr+(d-data);
		int rv=usb_control_msg(usbhdl,0x40,0xa0,
			/*addr=*/dl_addr,0,
			/*buf=*/(char*)d,/*size=*/bs,
			/*timeout=*/1000/*msec*/);
		if(rv<0)
		{  fprintf(stderr,"Writing %zu bytes at 0x%zx: %s\n",
			bs,dl_addr,usb_strerror());  ++n_errors;  }
		d+=bs;
	}
	
	return(n_errors);
}


int CypressFX2Device::ReadRAM(size_t addr,unsigned char *data,size_t nbytes)
{
	if(!IsOpen())
	{  fprintf(stderr,"ReadRAM: Not connected!\n");  return(1);  }
	
	int n_errors=0;
	
	const size_t chunk_size=16;
	
	unsigned char *d=data;
	unsigned char *dend=data+nbytes;
	while(d<dend)
	{
		size_t bs=dend-d;
		if(bs>chunk_size)  bs=chunk_size;
		size_t rd_addr=addr+(d-data);
		int rv=usb_control_msg(usbhdl,0xc0,0xa0,
			/*addr=*/rd_addr,0,
			/*buf=*/(char*)d,/*size=*/bs,
			/*timeout=*/1000/*msec*/);
		if(rv<0)
		{  fprintf(stderr,"Reading %zu bytes at 0x%zx: %s\n",
			bs,rd_addr,usb_strerror());  ++n_errors;  }
		d+=bs;
	}
	
	return(n_errors);
}


int CypressFX2Device::_ProgramIHexLine(const char *buf,
	const char *path,int line)
{
	const char *s=buf;
	if(*s!=':')
	{  fprintf(stderr,"%s:%d: format violation (1)\n",path,line);
		return(1);  }
	++s;
	
	unsigned int nbytes=0,addr=0,type=0;
	if(sscanf(s,"%02x%04x%02x",&nbytes,&addr,&type)!=3)
	{  fprintf(stderr,"%s:%d: format violation (2)\n",path,line);
		return(1);  }
	s+=8;
	
	if(type==0)
	{
		//printf("  Writing nbytes=%d at addr=0x%04x\n",nbytes,addr);
                assert(nbytes>0 && nbytes<256);
		unsigned char data[nbytes];
		unsigned char cksum=nbytes+addr+(addr>>8)+type;
		for(unsigned int i=0; i<nbytes; i++)
		{
			unsigned int d=0;
			if(sscanf(s,"%02x",&d)!=1)
			{  fprintf(stderr,"%s:%d: format violation (3)\n",path,line);
				return(1);  }
			s+=2;
			data[i]=d;
			cksum+=d;
		}
		unsigned int file_cksum=0;
		if(sscanf(s,"%02x",&file_cksum)!=1)
		{  fprintf(stderr,"%s:%d: format violation (4)\n",path,line);
			return(1);  }
		if((cksum+file_cksum)&0xff)
		{  fprintf(stderr,"%s:%d: checksum mismatch (%u/%u)\n",
			path,line,cksum,file_cksum);  return(1);  }
		if(WriteRAM(addr,data,nbytes))
		{  return(1);  }
	}
	else if(type==1)
	{
		// EOF marker. Oh well, trust it. 
		return(-1);
	}
	else
	{
		fprintf(stderr,"%s:%d: Unknown entry type %d\n",path,line,type);
		return(1);
	}

	return(0);
}


int CypressFX2Device::ProgramIHexFile(const char *path)
{
	if(!IsOpen())
	{  fprintf(stderr,"ProgramIHexFile: Not connected!\n");  return(1);  }
	
	FILE *fp=fopen(path,"r");
	if(!fp)
	{  fprintf(stderr,"Failed to open %s: %s\n",path,strerror(errno));
		return(2);  }
	
	int n_errors=0;
	
	const size_t buflen=1024;  // Hopefully much too long for real life...
	char buf[buflen];
	int line=1;
	for(;;++line)
	{
		*buf='\0';
		if(!fgets(buf,buflen,fp))
		{
			if(feof(fp))
			{  break;  }
			fprintf(stderr,"Reading %s (line %d): %s\n",path,line,
				strerror(ferror(fp)));
			fclose(fp);  fp=NULL;
			return(3);
		}
		
		int rv=_ProgramIHexLine(buf,path,line);
		if(rv<0)  break;
		if(rv)
		{  ++n_errors;  }
	}
	
	if(fp)
	{  fclose(fp);  }
	
	return(n_errors ? -1 : 0);
}


int CypressFX2Device::ProgramStaticIHex(const char **ihex)
{
	int n_errors=0;
	
	for(int line=0; ihex[line]; line++)
	{
		int rv=_ProgramIHexLine(ihex[line],"[builtin]",line+1);
		if(rv<0)  break;
		if(rv)
		{  ++n_errors;  }
	}
	
	return(n_errors ? -1 : 0);
}


int CypressFX2Device::ProgramBinFile(const char *path,size_t start_addr)
{
	if(!IsOpen())
	{  fprintf(stderr,"ProgramIHexFile: Not connected!\n");  return(1);  }
	
	int fd=::open(path,O_RDONLY);
	if(fd<0)
	{  fprintf(stderr,"Failed to open %s: %s\n",path,strerror(errno));
		return(2);  }
	
	int n_errors=0;
	const size_t buflen=1024;
	char buf[buflen];
	size_t addr=start_addr;
	for(;;)
	{
		ssize_t rd=read(fd,buf,buflen);
		if(!rd)  break;
		if(rd<0)
		{
			fprintf(stderr,"Reading %s: %s\n",path,strerror(errno));
			::close(fd);  fd=-1;
			return(3);
		}
		if(WriteRAM(addr,(const unsigned char*)buf,rd))
		{  ++n_errors;  }
		addr+=rd;
	}
	
	if(fd>=0)
	{  ::close(fd);  fd=-1;  }
	
	return(n_errors ? -1 : 0);
}


int CypressFX2Device::ProgramOpt9221BinFile(const char *path)
{

        unsigned char BitReverseTable256[] = {
                      0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
                      0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
                      0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
                      0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
                      0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
                      0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
                      0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
                      0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
                      0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
                      0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
                      0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
                      0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
                      0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
                      0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
                      0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
                      0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

        if(!IsOpen())
        {  fprintf(stderr,"Error - Program9221File: Not connected!\n");  return(1);  }

        int fd=::open(path,O_RDONLY);
        if(fd<0)
        {  fprintf(stderr,"Error - Failed to open %s: %s\n",path,strerror(errno));
                return(2);  }

        int n_errors=0, value = 0, index = 0;
        const size_t buflen=64;
        unsigned char buf[buflen], erasebuf[1] = {0x06};
        unsigned int startAddr = 0;

        // Configure the gpio to talk to the eeprom
        buf[0] = 0x06;
        if (CtrlMsgW(0x40, 0x17, 0 , 0, buf, 1))
        {
            ++n_errors;
        }

        // Write enable the eeprom
        buf[0] = 0x06;
        if (CtrlMsgW(0x40, 0x1E, 0 , 0, buf, 1))
        {
            ++n_errors;
        }

        // Check the write enable status
        if (CtrlMsgR(0xC0, 0x1B, 0 , 0, buf, 1))
        {
            ++n_errors;
        }
        if(buf[0]!=0x02)
        {
            fprintf(stderr,"Error - unable to set the EEPROM to write enabled. Status = 0x%02x\n",buf[0]);
            ++n_errors;
        }

        fprintf(stderr,"Erasing the EEPROM...\n");
        buf[0] = 0xC7;
        if (CtrlMsgW(0x40, 0x1E, 0 , 0, buf, 1))
        {
            ++n_errors;
        }

        // Wait for the chip erase to complete.
        for(int x = 30; x; --x)
        {
            // Check the erase status. 0x03 indicates busy 0x00 means complete.
            if (CtrlMsgR(0xC0, 0x1B, 0 , 0, buf, 1))
            {
                ++n_errors;
            }

            if(!buf[0])
            {
                fprintf(stderr,"  Erase complete.\n");
                break;
            }
            sleep(1);

            // See if we've timed out
            if(x==1)
            {
                fprintf(stderr,"Error - Chip erase timeout!\n");
                ++n_errors;
            }


        }

        fprintf(stderr,"Programming the opt9221 EEPROM...\n");
        while(!n_errors)
        {
                ssize_t rd=read(fd,buf,buflen);
                if(!rd)  break;
                if(rd<0)
                {
                        fprintf(stderr,"Reading %s: %s\n",path,strerror(errno));
                        ::close(fd);  fd=-1;
                        return(3);
                }
                for(int x = 0; x < rd; x++)
                {
                    buf[x] = BitReverseTable256[buf[x]];
                }

                value = (startAddr & 0xFFFF);
                index = (startAddr >> 16) & 0x00FF;

                nanosleep((const struct timespec[]){{0, 1000000L}}, NULL);


                // enable writes to the eeprom
                if (CtrlMsgW(0x40, 0x1E, 0 ,0, erasebuf, 1))
                {
                    ++n_errors;
                }

                // write the block of data to the eeprom
                if (CtrlMsgW(0x40, 0x18, value ,index, buf, rd))
                {
                    ++n_errors;
                }

                startAddr = startAddr + rd;
        }

        if(fd>=0)
        {  ::close(fd);  fd=-1;  }

        fprintf(stderr,"  Wrote %d bytes\n",startAddr);

        // Writing is complete now verify the image on the eeprom.
        if(!n_errors)
        {
            fprintf(stderr,"Verifying EEPROM program Vs source file...\n");

            // This reads the data from the eeprom and compares it to the source file.
            if(ReadProgramOpt9221(path, startAddr, 2))
            {
                fprintf(stderr,"Error - Verify failed. Programming failed.\n");
                ++n_errors;
            }
            else
            {
                fprintf(stderr,"  EEPROM and source match.\nProgramming operation was successful.\n");
            }
        }

        return(n_errors ? -1 : 0);
}

int CypressFX2Device::ReadProgramOpt9221(const char *path, unsigned int bytes, int mode)
{
/* Reads from the OPT9221 eeprom. Mode = 0 means the data read from the eeprom
 * is saved to a file in binary format. Mode = 1 means data read from the eeprom
 * is saved to a file in a human-readable text format. Mode = 2 means that data is
 * directly compared between a source file and the eeprom with an mismatches causing
 * this function to report an error.
 */
    unsigned char BitReverseTable256[] = {
                  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
                  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
                  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
                  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
                  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
                  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
                  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
                  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
                  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
                  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
                  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
                  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
                  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
                  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
                  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
                  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};


        if(!IsOpen())
        {  fprintf(stderr,"Read9221File: Not connected!\n");  return(1);  }

        int n_errors=0, value = 0, index = 0;
        const size_t buflen=64;
        unsigned char buf[buflen], obuf[buflen], rbuf[buflen];
        unsigned int startAddr = 0;
        FILE * fd = NULL;

        if(mode==2)
        {
            fd = fopen(path,"rb");
        }
        else if(mode==1)
        {
            fd = fopen(path,"w");
        }
        else
        {
            fd = fopen(path,"wb");
        }

        if(fd == NULL)
        {  fprintf(stderr,"Failed to open %s: %s\n",path,strerror(errno));
                return(2);
        }

        // Configure the eeprom for reading
        buf[0] = 0x06;
        if (CtrlMsgW(0x40, 0x17, 0 , 0, buf, 1))
        {
            ++n_errors;
        }

        while(startAddr < bytes)
        {
            value = (startAddr & 0xFFFF);
            index = (startAddr >> 16) & 0x00FF;

            if(CtrlMsgR(0xC0, 0x19, value, index, buf, buflen))
            {
                ++n_errors;
            }

            for(unsigned int x = 0; x < buflen; x++)
            {
                obuf[x] = BitReverseTable256[buf[x]];
            }

            if(mode==2)
            {
                int bytes_read = fread(rbuf, sizeof(char), buflen, fd);
                for(int x = 0; x < bytes_read; x++)
                {
                    if(rbuf[x]!=obuf[x])
                    {
                        fprintf(stderr,"Error - Bytes comparison failed 0x%02x from file vs 0x%02x from EEPROM.\n", rbuf[x], obuf[x]);
                        ++n_errors;
                    }
                }
            }
            else if(mode==1)
            {
                for(size_t x = 0; x < buflen; x++)
                {
                    fprintf(fd,"0x%02x ", obuf[x]);
                    if((x==15) || (x==31) || (x==47))
                    {
                        fprintf(fd,"\r\n");
                    }
                }
                fprintf(fd,"\r\n");
            }
            else
            {
                int bytes_written = fwrite(obuf, sizeof(char), buflen, fd);

                if(bytes_written != buflen)
                {
                    ++n_errors;
                }
            }

            if(n_errors > 9 )
            {
                // Too many errors, let's get out of here.
                break;
            }

            startAddr = startAddr + buflen;
        }

        if(mode!=2)
        {
            fprintf(stderr,"  Read %d bytes\n",startAddr);
        }

        if(fd != NULL)
        {  ::fclose(fd);  fd=NULL;  }

        return(n_errors ? -1 : 0);
}

int CypressFX2Device::Dump9221Registers(const char *path, int mode)
{
/* Dumps the OPT9221 register contenets to a text file
 */
        unsigned char DE_register_map[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08
                                          ,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11
                                          ,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a
                                          ,0x1b,0x1f,0x25,0x27,0x28,0x29,0x2e,0x2f,0x30
                                          ,0x31,0x33,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b
                                          ,0x3c,0x3d,0x3e,0x3f,0x40,0x47,0x48,0x4c,0x4d
                                          ,0x51,0x52,0x61,0x62,0x63,0x65,0x66,0x80,0x81
                                          ,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x91,0x92
                                          ,0x93,0x94,0x95,0x96,0x97,0x98,0xab,0xac,0xad
                                          ,0xae,0xaf,0xb0,0xb1,0xb2,0xb3,0xb6};

        unsigned char TG_register_map[] = {0x02,0x0b,0x0c,0x0d,0x0e,0x0f,0x12,0x1f,0x20
                                          ,0x21,0x22,0x80,0x81,0x82,0x83,0xcc,0xd6};


        if(!IsOpen())
        {  fprintf(stderr,"ReadFX2Eeprom: Not connected!\n");  return(1);  }

        int n_errors=0;
        const size_t buflen=64;
        unsigned char buf[buflen];
        FILE * fd = NULL;

        if(mode==1)
        {
            fd = fopen(path,"w");
            if(fd == NULL)
            {  fprintf(stderr,"Failed to open %s: %s\n",path,strerror(errno));
                    return(2);
            }
        }

        // Read in the DE set of registers
        for(int x=0; x < 88; ++x)
        {
            if(CtrlMsgR(0xC0, 0x08, 0x5c, DE_register_map[x], buf,3))
            {
                ++n_errors;
            }

            if(mode==1)
            {
                fprintf(fd,"0x5C%02x = 0x%02x%02x%02x\r\n ",(unsigned int)DE_register_map[x],(unsigned int)buf[2],(unsigned int)buf[1],(unsigned int)buf[0]);
            }
            else
            {
                printf("0x5C%02x = 0x%02x%02x%02x\r\n ",(unsigned int)DE_register_map[x],(unsigned int)buf[2],(unsigned int)buf[1],(unsigned int)buf[0]);
            }

            if(n_errors > 9 )
            {
                // Too many errors, let's get out of here.
                break;
            }


        }

        // Read in the TG set of registers
        for(int x=0; x < 17; ++x)
        {
            if(CtrlMsgR(0xC0, 0x08, 0x58, TG_register_map[x], buf,3))
            {
                ++n_errors;
            }

            if(mode==1)
            {
                    fprintf(fd,"0x58%02x = 0x%02x%02x%02x\r\n ",(unsigned int)TG_register_map[x],(unsigned int)buf[2],(unsigned int)buf[1],(unsigned int)buf[0]);
            }
            else
            {
                    printf("0x58%02x = 0x%02x%02x%02x\r\n ",(unsigned int)TG_register_map[x],(unsigned int)buf[2],(unsigned int)buf[1],(unsigned int)buf[0]);
            }

            if(n_errors > 9 )
            {
                // Too many errors, let's get out of here.
                break;
            }


        }


        if(fd != NULL)
        {  ::fclose(fd);  fd=NULL;  }

        return(n_errors ? -1 : 0);
}

int CypressFX2Device::ProgramFx2BinFile(const char *path)
{
        if(!IsOpen())
        {  fprintf(stderr,"Error - ProgramFx2File: Not connected!\n");  return(1);  }

        int fd=::open(path,O_RDONLY);
        if(fd<0)
        {  fprintf(stderr,"Error - Failed to open %s: %s\n",path,strerror(errno));
                return(2);  }

        int n_errors=0, value = 0;
        const size_t buflen=64;
        unsigned char buf[buflen];
        unsigned int startAddr = 0;

        fprintf(stderr,"Programming the optFX2 EEPROM...\n");
        while(!n_errors)
        {
                ssize_t rd=read(fd,buf,buflen);
                if(!rd)  break;
                if(rd<0)
                {
                        fprintf(stderr,"Reading %s: %s\n",path,strerror(errno));
                        ::close(fd);  fd=-1;
                        return(3);
                }

                value = (startAddr & 0xFFFF);
                nanosleep((const struct timespec[]){{0, 1000000L}}, NULL);

                // write the block of data to the eeprom
                if (CtrlMsgW(0x40, 0x35, 0x0000 , value, buf, rd))
                {
                    ++n_errors;
                }

                startAddr = startAddr + rd;
        }

        if(fd>=0)
        {  ::close(fd);  fd=-1;  }

        fprintf(stderr,"  Wrote %d bytes\n",startAddr);

        return(n_errors ? -1 : 0);
}

int CypressFX2Device::ReadFX2Eeprom(const char *path, unsigned int bytes, int mode)
{
/* Reads from the FX2 eeprom. Mode = 0 means the data read from the eeprom
 * is saved to a file in binary format. Mode = 1 means data read from the eeprom
 * is saved to a file in a human-readable text format.
 */

        if(!IsOpen())
        {  fprintf(stderr,"ReadFX2Eeprom: Not connected!\n");  return(1);  }

        int n_errors=0, value = 0;
        const size_t buflen=64;
        unsigned char buf[buflen];
        unsigned int startAddr = 0, bytes_to_write;
        FILE * fd = NULL;

        if(mode==1)
        {
            fd = fopen(path,"w");
        }
        else
        {
            fd = fopen(path,"wb");
        }

        if(fd == NULL)
        {  fprintf(stderr,"Failed to open %s: %s\n",path,strerror(errno));
                return(2);
        }

        while(bytes)
        {
            value = (startAddr & 0xFFFF);

            if(bytes > buflen)
            {
                bytes_to_write = buflen;
            }
            else
            {
                bytes_to_write = bytes;
            }

            // Read from the FX2's EEPROM
            if(CtrlMsgR(0xC0, 0x07, 0, value, buf, bytes_to_write))
            {
                ++n_errors;
            }


            if(mode==1)
            {
                for(size_t x = 0; x < bytes_to_write; x++)
                {
                    fprintf(fd,"0x%02x ", buf[x]);
                    if((x==15) || (x==31) || (x==47))
                    {
                        fprintf(fd,"\r\n");
                    }
                }
                fprintf(fd,"\r\n");
            }
            else
            {
                unsigned int bytes_written = fwrite(buf, sizeof(char), bytes_to_write, fd);

                if(bytes_written != bytes_to_write)
                {
                    ++n_errors;
                }
            }

            if(n_errors > 9 )
            {
                // Too many errors, let's get out of here.
                break;
            }

            startAddr = startAddr + buflen;
            bytes -= bytes_to_write;
        }

        if(mode!=2)
        {
            fprintf(stderr,"  Read %d bytes\n",startAddr);
        }

        if(fd != NULL)
        {  ::fclose(fd);  fd=NULL;  }

        return(n_errors ? -1 : 0);
}

int CypressFX2Device::SerialNumberRead(const unsigned char *ctl_buf, size_t ctl_buf_size)
{
        if(!IsOpen())
        {  fprintf(stderr,"GetSerial: Not connected!\n");  return(1);  }

        int rv=usb_control_msg(usbhdl,0xC0,0x32,0,0, (char*)ctl_buf,ctl_buf_size, /*timeout=*/1000/*msec*/);
        if(rv<0)
        {
            fprintf(stderr,"Error reading serial number: %s\n", usb_strerror());
        }

        return(rv);
}

int CypressFX2Device::SerialNumberWrite(const unsigned char *ctl_buf, size_t ctl_buf_size)
{
        if(!IsOpen())
        {  fprintf(stderr,"SetSerial: Not connected!\n");  return(1);  }

        int rv=usb_control_msg(usbhdl,0x40,0x32,0,0, (char*)ctl_buf,ctl_buf_size, /*timeout=*/1000/*msec*/);
        if(rv<0)
        {
            fprintf(stderr,"Error writing serial number: %s\n", usb_strerror());
        }

        return(rv);
}

int CypressFX2Device::OrionSerialNumberRead(const unsigned char *ctl_buf, size_t ctl_buf_size)
{
        // Serial number format is 1 byte length, 1 to 64 bytes serial
        // number value (ASCII), 2 bytes CRC-16 CCITT

        int length = 0;
        unsigned int address = 0;
        unsigned short stored_checksum = 0xFFFF;

        if(!IsOpen())
        {  fprintf(stderr,"GetSerial: Not connected!\n");  return(1);  }

        // Get the serial number length
        int bytes_read = usb_control_msg(usbhdl,0xC0,0x04,0x5200,0, (char*)&length,1, /*timeout=*/1000/*msec*/);
        if(bytes_read != 1)
        {
            fprintf(stderr,"Error: Reading serial number length!\n");
            return(-1);
        }
        ++address;

        // Limit the maximum length in the case of an unprogrammed board
        if(length > 64)
        {
            length = 64;
        }
        // Make sure we don't exceed the supplied buffer
        if(length > (int)ctl_buf_size)
        {
            return(-1);
        }

        // Read the serial number from EEPROM
        for(int x = 0; x < length; ++x)
        {
            // Only one byte reads are supported by the TI FX2 firmware for this type of EEPROM
            // address format is LSBMSB
            bytes_read = usb_control_msg(usbhdl,0xC0,0x04,0x5200,address << 8, (char*)&ctl_buf[x],1, /*timeout=*/1000/*msec*/);
            if(bytes_read != 1)
            {
                fprintf(stderr,"Error: Reading serial number value!\n");
                return(-1);
            }
            ++address;
        }
        // Read in the 2-byte CRC
        char * b_stored_checksum = (char *)&stored_checksum;
        for(unsigned int x = 0; x < 2; ++x)
        {
            bytes_read = usb_control_msg(usbhdl,0xC0,0x04,0x5200,address << 8, &b_stored_checksum[x],1, /*timeout=*/1000/*msec*/);
            if(bytes_read != 1)
            {
                fprintf(stderr,"Error: Reading the stored checksum!\n");
                return(-1);
            }
            ++address;
        }

        // Check the CRC calculated vs read from EEPROM
        unsigned short read_checksum = Computecrc16(ctl_buf, length);
        if(stored_checksum != read_checksum)
        {
            fprintf(stderr,"Error: Checksums do not match %X read vs %X stored!\n", stored_checksum, read_checksum);
            return(-1);
        }

        return(length);
}

int CypressFX2Device::OrionSerialNumberWrite(const unsigned char *ctl_buf, size_t ctl_buf_size)
{
        // Serial number format is 1 byte length, 1 to 64 bytes serial
        // number value (ASCII), 2 bytes CRC-16 CCITT

        int length = (int)ctl_buf_size;
        int x = 0;
        unsigned int address = 0;
        unsigned short checksum = 0;

        if(!IsOpen())
        {  fprintf(stderr,"SetSerial: Not connected!\n");  return(1);  }

        // Write the serial number length
        int bytes_written = usb_control_msg(usbhdl,0x40,0x03,0x5200,0, (char*)&length,1, /*timeout=*/1000/*msec*/);
        if(bytes_written != 1)
        {
            fprintf(stderr,"Error: Writing the serial number length!\n");
            return(-1);
        }
        // Sleep to allow the write to EEPROM time to complete
        nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);
        ++address;

        if(length>64)
            length = 64;

        bytes_written = 0;

        for(x = 0; x < length; x++)
        {
            // Only one byte writes are supported by the TI FX2 firmware for this type of EEPROM
            bytes_written += usb_control_msg(usbhdl,0x40,0x03,0x5200, address << 8, (char*)&ctl_buf[x],1, /*timeout=*/1000/*msec*/);
            nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);
            ++address;
        }

        if(bytes_written != length)
        {
            fprintf(stderr,"Error: Writing the serial number value %d bytes written!\n", bytes_written);
            return(-1);
        }

        // Use a crc16 CCITT checksum method to generate a 2 byte CRC
        checksum = Computecrc16(ctl_buf, length);
        char b_checksum[2];
        b_checksum[0] = (char) (checksum & 0xFF);
        b_checksum[1] = (char) ((checksum >> 8) & 0xFF);

        // write the two byte checksum
        usb_control_msg(usbhdl,0x40,0x03,0x5200, address << 8, &b_checksum[0],1, /*timeout=*/1000/*msec*/);
        nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);
        ++address;
        usb_control_msg(usbhdl,0x40,0x03,0x5200, address << 8, &b_checksum[1],1, /*timeout=*/1000/*msec*/);
        nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);

        return(bytes_written);
}

unsigned short CypressFX2Device::Computecrc16(const unsigned char* data_p, unsigned char length)
{
        // Computes a CRC-16 CCITT 0xFFFF
        // Values generated can be checked at https://www.lammertbies.nl/comm/info/crc-calculation.html
        unsigned char x;
        unsigned short crc = 0xFFFF;

        while (length--){
            x = crc >> 8 ^ *data_p++;
            x ^= x>>4;
            crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
        }

        return crc;
}

int CypressFX2Device::CtrlMsg(unsigned char requesttype,
	unsigned char request,int value,int index,
	const unsigned char *ctl_buf,size_t ctl_buf_size)
{
	if(!IsOpen())
	{  fprintf(stderr,"CtrlMsg: Not connected!\n");  return(1);  }
	
	int n_errors=0;
	
	int rv=usb_control_msg(usbhdl,requesttype,request,
		value,index,
		(char*)ctl_buf,ctl_buf_size,
		/*timeout=*/1000/*msec*/);
	if(rv<0)
	{  fprintf(stderr,"Sending USB control message: %s\n",
		usb_strerror());  ++n_errors;  }
	
	return(n_errors);
}


int CypressFX2Device::CtrlMsgW(unsigned char requesttype,
        unsigned char request,int value,int index,
        const unsigned char *ctl_buf,size_t ctl_buf_size)
{
        if(!IsOpen())
        {  fprintf(stderr,"CtrlMsg: Not connected!\n");  return(1);  }

        int n_errors=0;

        int rv=usb_control_msg(usbhdl,requesttype,request,
                value,index,
                (char*)ctl_buf,ctl_buf_size,
                /*timeout=*/1000/*msec*/);
        if(rv<0)
        {  fprintf(stderr,"Sending USB control message: %s\n",
                usb_strerror());  ++n_errors;  }

        return(n_errors);
}

int CypressFX2Device::CtrlMsgR(unsigned char requesttype,
        unsigned char request,int value,int index,
        const unsigned char *ctl_buf,size_t ctl_buf_size)
{
        if(!IsOpen())
        {  fprintf(stderr,"CtrlMsg: Not connected!\n");  return(1);  }

        int n_errors=0;

        int rv=usb_control_msg(usbhdl,requesttype,request,
                value,index,
                (char*)ctl_buf,ctl_buf_size,
                /*timeout=*/1000/*msec*/);
        if(rv<0)
        {  fprintf(stderr,"Sending USB control message: %s\n",
                usb_strerror());  ++n_errors;  }

        return(n_errors);
}


int CypressFX2Device::open(struct usb_device *_usbdev)
{
	close();
	usbdev=_usbdev;
	usbhdl=usb_open(usbdev);
	if(!usbhdl)
	{  fprintf(stderr,"Failed to open device: %s\n",usb_strerror());
		return(1);  }
	return(0);
}


int CypressFX2Device::close()
{
	int rv=0;
	if(usbhdl)
	{
		rv=usb_close(usbhdl);  usbhdl=NULL;
		if(rv)
		{  fprintf(stderr,"closing USB device: %s\n",usb_strerror());  }
	}
	usbdev=NULL;
	return(rv);
}


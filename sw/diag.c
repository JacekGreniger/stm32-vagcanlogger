#include "stm32f10x_lib.h"
#include "filesystem/integer.h"
#include "filesystem/diskio.h"
#include "filesystem/ff.h"
#include "kw1281.h"
#include "vwtp.h"
#include "config.h"

extern vu16 timeSec;
extern vu8 time10MSec;

u8 Byte2HexStr(u8 * s, u8 val)
{
  const u8 * hexArray = "0123456789abcdef";
  s[0] = hexArray[val/16];
  s[1] = hexArray[val%16];
  return 2;
}


u8 DecodeUnits(u8 *p, u8 *frameData)
{
  u8 *p_saved = p;
  int i;
  u8 len;
  
  for (i=0; i<12; i+=3)
  {
    switch (frameData[i])
    {
      case 1: 
        p += sprintf(p, "rpm");
        break;

      case 2: 
      case 20: 
      case 23:
      case 33:
        p += sprintf(p, "%%");
        break;

      case 3: 
      case 9: 
      case 27: 
      case 34: 
      case 67:
        p += sprintf(p, "deg");
        break;

      case 5: 
      case 31:
        p += sprintf(p, "°C");
        break;
        
      case 6: 
      case 21:
      case 43:
      case 66:
        p += sprintf(p, "V");
        break;
        
      case 7: 
        p += sprintf(p, "km/h");
        break;

      case 12: 
      case 64:
        p += sprintf(p, "ohm");
        break;

      case 13: 
      case 65:
        p += sprintf(p, "mm");
        break;

      case 14: 
      case 69:
        p += sprintf(p, "bar");
        break;

      case 15:
      case 22:
      case 47:
        p += sprintf(p, "ms");
        break;
        
      case 16: 
        p += sprintf(p, "bitval");
        break;
        
      case 18: 
      case 50: 
        p += sprintf(p, "mbar");
        break;
                
      case 19: 
        p += sprintf(p, "l");
        break;
                
      case 24:
      case 40:
        p += sprintf(p, "A");
        break;
             
      case 25: 
      case 53: 
        p += sprintf(p, "g/s");
        break;
                
      case 26: 
        p += sprintf(p, "C");
        break;
                        
      case 35: 
        p += sprintf(p, "l/h");
        break;

      case 36: 
        p += sprintf(p, "km");
        break;

      case 37: 
        p += sprintf(p, "state");
        break;
      
      case 30:
      case 38:
      case 46: 
        p += sprintf(p, "deg k/w");
        break;

      case 41: 
        p += sprintf(p, "Ah");
        break;

      case 42: 
        p += sprintf(p, "Kw");
        break;

      case 44: 
        p += sprintf(p, "h:m");
        break;

      case 39:
      case 49: 
      case 51:
        p += sprintf(p, "mg/h");
        break;
        
      case 52: 
        p += sprintf(p, "Nm");
        break;
       
      case 54: 
        p += sprintf(p, "count");
        break;
       
      case 55: 
        p += sprintf(p, "s");
        break;
      
      case 56: 
      case 57:
        p += sprintf(p, "WSC");
        break;
              
      case 60: 
        p += sprintf(p, "sec");
        break;

      case 62: 
        p += sprintf(p, "S");
        break;
      
      case 68: 
        p += sprintf(p, "deg/s");
        break;

      case 70: 
        p += sprintf(p, "m/s^2");
        break;

      default:
        break;
    }
    *p++ = ';';
  }

  len = p - p_saved;
  return len;
}


u8 DecodeFrame(u8 *p, u8 *frameData)
{
  u16 val_u16;
  s16 val_s16;
  float f;
  int i, j, len;
  u8 *p_saved = p;
  
  for (i=0; i<12; i+=3)
  {
    switch (frameData[i])
    {
    
      case 1: //0.2*a*b rpm
        val_u16 = frameData[i+1] * frameData[i+2];
        val_u16 /= 5;
        p += sprintf(p, "%d", val_u16);
        break;

      case 2: //a*0.002*b  	%
      case 3: //0.002*a*b  	Deg
        f = frameData[i+1] * frameData[i+2];
        f *= 0.002;
        p += sprintf(p, "%.1f", f);
        break;

      case 4: // abs(b-127)*0.01*a  	"ATDC" if Value >127, else"BTDC"
        val_u16 = abs(frameData[i+2] - 127);
        val_u16 *= frameData[i+1];
        val_u16 /= 100;
        p += sprintf(p, "%d", val_u16);
        break;
                
      case 5: // a*(b-100)*0.1 °C
        val_s16 = frameData[i+1] * (frameData[i+2] - 100);
        val_s16 /= 10;
        p += sprintf(p, "%d", val_s16);
        break;
      
      case 6:  //0.001*a*b  	V
      case 12: //0.001*a*b  	Ohm
      case 21: //0.001*a*b  	V
      case 22: //0.001*a*b  	ms
      case 24: //0.001*a*b  	A
        f = frameData[i+1] * frameData[i+2];
        f *= 0.001;
        p += sprintf(p, "%.2f", f);
        break;
      
      case 7: // 0.01*a*b km/h
        val_u16 = frameData[i+1] * frameData[i+2];
        val_u16 /= 100;
        p += sprintf(p, "%d", val_u16);
        break;
        
      case 8: //0.1*a*b  	(no units)
        f = frameData[i+1] * frameData[i+2];
        f *= 0.1;
        p += sprintf(p, "%.1f", f);
        break;

      case 9: //(b-127)*0.02*a  	Deg
        f = (frameData[i+2] - 127.0) * frameData[i+1];
        f *= 0.02;
        p += sprintf(p, "%.1f", f);
        break;
        
      case 10:
        if (0 == frameData[i+2])
        {
          p += sprintf(p, "COLD");
        }
        else
        {
          p += sprintf(p, "WARM");
        }
        break;
        
      case 11: //0.0001*a*(b-128)+1  -
        f = frameData[i+1] * (frameData[i+2] - 128.0);
        f *= 0.0001;
        f += 1;
        p += sprintf(p, "%.1f", f);
        break;

      case 13: //(b-127)*0.001*a  	mm
        f = (frameData[i+2] - 127.0) * frameData[i+1];
        f *= 0.001;
        p += sprintf(p, "%.1f", f);
        break;

      case 14: //0.005*a*b  	bar
        f = frameData[i+2] * frameData[i+1];
        f *= 0.005;
        p += sprintf(p, "%.1f", f);
        break;
        
      case 15: //0.01*a*b ms
      case 19: //a*b*0.01  	l
        f = frameData[i+1] * frameData[i+2];
        f *= 0.01;
        p += sprintf(p, "%.2f", f);
        break;
        
      case 16: //bitvalue
        for (j = 128; j > 0; j = j>>1)
        {
          if (frameData[i+1] & j)
          {
            *p++ = (frameData[i+2] & j)?'1':'0';
          }
          else
          {
            *p++ = 'X';
          }
        }
        break;
        
      case 17: //chr(a) + chr(b) 
        p += sprintf(p, "%c%c", frameData[i+1], frameData[i+2]);
        break;
        
      case 18:  //0.04*a*b  mbar
        val_u16 = frameData[i+1] * frameData[i+2];
        val_u16 /= 25;
        p += sprintf(p, "%d", val_u16);
        break;
        
      case 20:  //	a*(b-128)/128  	%
        f = frameData[i+1] * (frameData[i+2] - 128);
        f /= 128.0;
        p += sprintf(p, "%.2f", f);
        break;
        
      case 23: //23  	EGR Valve, Duty Cycle / Inj. Timing ???  	   	b/256*a  	%
        f = frameData[i+2] / 256.0;
        f *= frameData[i+1];
        p += sprintf(p, "%.1f", f);
        break;
        
      case 25: //  	(b*1.421)+(a/182)  	g/s
        f = (frameData[i+1] / 182.0) + (1.421 * frameData[i+2]);
        p += sprintf(p, "%.2f", f);
        break;

      case 26: //b-a  	C
      case 28:
        val_s16 = frameData[i+2] - frameData[i+1];
        p += sprintf(p, "%d", val_s16);
        break;
       
      case 27: //abs(b-128)*0.01*a 
        f = (frameData[i+2] - 128.0);
        if (f < 0)
        {
          f = f * (-1);
        }
        f *= frameData[i+1];
        f *= 0.01;
        p += sprintf(p, "%.1f", f);
        break;

      case 30: //b/12*a  	Deg k/w
        f = frameData[i+2] / 12.0;
        f *= frameData[i+1];
        p += sprintf(p, "%.1f", f);
        break;

      case 31: //b/2560*a  	°C
        f = frameData[i+2] / 2560.0;
        f *= frameData[i+1];
        p += sprintf(p, "%.1f", f);
        break;
        
      case 33: // 100*b/a  (if a==0 then 100*b)  	%
        if (frameData[i+1] == 0)
        {
          val_u16 = 100 * frameData[i+2];
        }
        else
        {
          val_u16 = (100 * frameData[i+2])/frameData[i+1];
        }
        p += sprintf(p, "%d", val_u16);
        break;
      
      case 34: // (b-128)*0.01*a  	kW
        f = (frameData[i+2] - 128.0) * frameData[i+1];
        f *= 0.01;
        p += sprintf(p, "%.1f", f); 
        break;
        
      case 35: // 0.01*a*b  	l/h
        f = frameData[i+2] * frameData[i+1];
        f *= 0.01;
        p += sprintf(p, "%.1f", f); 
        break;        

      case 37:
        switch (frameData[i+2])
        {
          case 0x00:
            p += sprintf(p, "-");
            break;

          case 0x02:
            p += sprintf(p, "ADP OK"); 
            break;
                      
          case 0x05:
            p += sprintf(p, "Idle"); 
            break;
            
          case 0x06:
            p += sprintf(p, "Partial thr"); 
            break;
            
          case 0x07:
            p += sprintf(p, "WOT"); 
            break;

          case 0x08:
            p += sprintf(p, "Enrichment"); 
            break;

          case 0x09:
            p += sprintf(p, "Deceleration"); 
            break;

          case 0x0E:
            p += sprintf(p, "A/C low"); 
            break;

          case 0x10:
            p += sprintf(p, "Compr. OFF"); 
            break;

          case 0xD6:
            p += sprintf(p, "Htg. S1 0xD6"); 
            break;

          case 0xD7:
            p += sprintf(p, "Htg. S1 0xD7"); 
            break;

          case 0xD9:
            p += sprintf(p, "Htg. S2 0xD9"); 
            break;

          case 0xEB:
            p += sprintf(p, "Test OFF"); 
            break;

          default:
            p += sprintf(p, "0x%02x", frameData[i+2]); 
        }
        break;

      case 38: //(b-128)*0.001*a  	Deg k/w
        f = (frameData[i+2] - 128.0);
        f *= frameData[i+1];
        f *= 0.001;
        p += sprintf(p, "%.1f", f); 
        break;
              
      case 39: //b/256*a  mg/h
        f = frameData[i+2] / 256.0;
        f *= frameData[i+1];
        p += sprintf(p, "%.1f", f); 
        break;

      case 43: // 	b*0.1+(25.5*a)  	V
        f = frameData[i+2] * 0.1;
        f += 25.5 * frameData[i+1];
        p += sprintf(p, "%.2f", f); 
        break;
              
      case 44: //a : b  	h:m
        p += sprintf(p, "%d:%d", frameData[i+1], frameData[i+2]);
        break;        

      case 45: //0.1*a*b/100  	 
        f = frameData[i+2] * frameData[i+1];
        f /= 1000.0;
        p += sprintf(p, "%.3f", f); 
        break;

      case 46: //(a*b-3200)*0.0027  	Deg k/w
        f = frameData[i+2] * frameData[i+1];
        f -= 3200.0;
        f *= 0.0027;
        p += sprintf(p, "%.1f", f); 
        break;

      case 47: //(b-128)*a  	ms
        val_s16 = (frameData[i+2] - 128) * frameData[i+1];
        p += sprintf(p, "%d", val_s16); 
        break;

      case 49: //(b/4)*a*0,1  mg/h
        f = (frameData[i+2] / 4.0);
        f *= frameData[i+1];
        f *= 0.1;
        p += sprintf(p, "%.1f", f); 
        break;
      
      case 50: // (b-128)/(0.01*a), if a==0 (b-128)/0.01  	mbar
        f = (frameData[i+2] - 128.0);
        f /= 0.01;
        if (frameData[i+1] != 0)
        {
          f /= frameData[i+1];
        }
        p += sprintf(p, "%.1f", f); 
        break;

      case 51: //((b-128)/255)*a  	mg/h
        f = frameData[i+2] - 128.0;
        f /= 255.0;
        f *= frameData[i+1];
        p += sprintf(p, "%.1f", f); 
        break;

      case 52: // b*0.02*a-a  	Nm
        val_u16 = frameData[i+1] * frameData[i+2];
        val_u16 /= 50;
        val_u16 -= frameData[i+1];
        p += sprintf(p, "%d", val_u16);
        break;

      case 53: // 53  	Luftdurchfluß Luftmassenmesser (???)  	   	(b-128)*1.4222+0.006*a  	g/s
        f = (frameData[i+2] - 128.0) * 1.4222;
        f += frameData[i+1] * 0.006;
        p += sprintf(p, "%.1f", f); 
        break;

      case 48: //b+a*255  	-      
      case 54: // 	a*256+b  	Count
        val_u16 = frameData[i+1] * 256;
        val_u16 += frameData[i+2];
        p += sprintf(p, "%d", val_u16);
        break;

      case 55: //a*b/200  	s
        f = frameData[i+1] * frameData[i+2];
        f /= 200.0;
        p += sprintf(p, "%.1f", f);
        break;

      case 56: //a*256+b  	WSC
        val_u16 = 256 * frameData[i+1] + frameData[i+2];
        p += sprintf(p, "%d", val_u16);
        break;

      case 59: //(a*256+b)/32768  	-
        f = 256.0 * frameData[i+1] + frameData[i+2];
        f = 32768.0;
        p += sprintf(p, "%.2f", f);
        break;

      case 60: //(a*256+b)*0.01  	sec
        f = 256.0 * frameData[i+1] + frameData[i+2];
        f *= 0.01;
        p += sprintf(p, "%.1f", f);
        break;

      case 61: // (b-128)/a, if a==0 (b-128)  -
        f = frameData[i+2] - 128.0;
        if (frameData[i+1] != 0)
        {
          f /= frameData[i+1];
        }
        p += sprintf(p, "%.2f", f); 
        break;

      case 62: //0.256*a*b  	S
        f = frameData[i+1] * frameData[i+2];
        f *= 0.256;
        p += sprintf(p, "%.1f", f);
        break;
        
      case 63: //chr(a) + chr(b) + "?"  -
        p += sprintf(p, "%c%c?", frameData[i+1], frameData[i+2]);
        break;

      case 64: //a+b  	Ohm
        val_u16 = frameData[i+1] + frameData[i+2];
        p += sprintf(p, "%d", val_u16);
        break;

      case 65: //0.01*a*(b-127)  	mm
        f = 0.01 * frameData[i+1];
        f *= frameData[i+2] - 127.0;
        p += sprintf(p, "%.2f", f);
        break;

      case 66: //(a*b)/511.12  	V
        f = frameData[i+1] * frameData[i+2];
        f /= 511.12;
        p += sprintf(p, "%.2f", f);
        break;

      case 67: //(640*a)+b*2.5  	Deg
        f = 640.0 * frameData[i+1];
        f += 2.5 * frameData[i+2];
        p += sprintf(p, "%.1f", f);
        break;

      case 68: //(256*a+b)/7.365  	deg/s
        f = 256.0 * frameData[i+1] + frameData[i+2];
        f /= 7.365;
        p += sprintf(p, "%.2f", f);
        break;

      case 69: //(256*a +b)*0.3254  	Bar
        f = 256.0 * frameData[i+1] + frameData[i+2];
        f *= 0.3254;
        p += sprintf(p, "%.2f", f);
        break;

      case 70: //(256*a +b)*0.192  	m/s^2
        f = 256.0 * frameData[i+1] + frameData[i+2];
        f *= 0.192;
        p += sprintf(p, "%.2f", f);
        break;

      default:
        *p++ = '0';
        *p++ = 'x';
        p += Byte2HexStr(p, frameData[i]);
        p += Byte2HexStr(p, frameData[i+1]);
        p += Byte2HexStr(p, frameData[i+2]);
    }
    *p++ = ';';
  }

  len = p - p_saved;
  return len;
}



// 0 - terminated by user
// 1 - communication error
// 2 - fat error
int kw1281_diag( u8 *config, FIL *log_file, u8 kw1281Debug) 
{
  KW1281struct_t frame;
#ifdef DEBUG_LOG
  u8 stage = 0;
  KW1281struct_t debug_frame;
#endif //DEBUG_LOG  
  u8 KW1281BlockCounter;
  u8 KW1281data[128];
  u8 result = 0;
  u8 sendRequest = 0;
  u8 cnt = 0;
  int errorCode = 0;
  u8 groupNumber = 0;
  u8 groupPos = 0;
  u8 requestedGroup = 0;
  u8 currentLogLine[150];
  u8 *p;
#ifdef DEBUG_LOG
  u8 buf[250];
  u8 *buf_p = buf;
#endif //DEBUG_LOG
  u8 units_buf[150];
  u8 *units_buf_p;
  u16 i, j;
  u8 file_header[120];

  units_buf_p = units_buf;
  units_buf_p += sprintf(units_buf_p, "s;");
  
  p = file_header;
  p += sprintf(p, "time;");
  for (i=1; i <= config[0]; i++)
  {
    for (j=1; j <= 4; j++)
    {
      p += sprintf(p,"%d %d;", config[i], j);
    }
  }
  p += sprintf(p, "\n");

  frame.data = KW1281data;
  p = currentLogLine;

  while (1) // petla po nawiazaniu polaczenia z ECU
  {
    frame.title=0xff;

    // odebranie ramki od ECU
    result = KW1281ReceiveBlock(&frame);
    if (result) 
    {
    #ifdef DEBUG_LOG    
      stage = 1;
    #endif
      errorCode = 1;
      break;
    }  
    
    KW1281BlockCounter = frame.cnt;
    sendRequest = 0;

    if (0 != buttonState) //key pressed, end of session
    {
      frame.len = 3;
      frame.title = KW1281_END_OF_SESSION;
      KW1281BlockCounter++; // inkrementacja lokalnie przechowywanej wartosci Block Counter
      frame.cnt = KW1281BlockCounter;     
      result = KW1281SendBlock(&frame); //wyslanie bloku do ECU 
      break;
    }
    
    // odebranie bloku ACK - mozna wyslac nowa komende 
    if (KW1281_ACK == frame.title)
    { 
      if (cnt > 8)
      {
        sendRequest = 1;

        //write header to log file
        if ((f_puts("-------------\n\n", log_file) < 0) ||
            (f_puts(file_header, log_file) < 0))
        {
          errorCode = 2;
          break;
        }

        groupNumber = config[1];
        groupPos = 1;
        p = currentLogLine;
        p += sprintf(p, "%d.%02d;", timeSec, time10MSec);
      #ifdef DEBUG_LOG
        if (kw1281Debug)
        {
          buf_p = buf;
          buf_p += sprintf(buf_p, "%d.%02d;", timeSec, time10MSec);
        }
      #endif // DEBUG_LOG
      }
      else 
      {
        ++cnt;
      }
    }
    else if (KW1281_GROUP_RESP == frame.title)
    {
      p += DecodeFrame(p, frame.data);
      
      if (units_buf_p)
      {
        units_buf_p += DecodeUnits(units_buf_p, frame.data);
      }
    #ifdef DEBUG_LOG
      if (kw1281Debug)
      {
        for (i=0; i<12; i++)
        {
          buf_p += Byte2HexStr(buf_p, frame.data[i]);
          if (2 == (i%3))
          {
            *buf_p++ = ';';
          }
        }
      }
    #endif //DEBUG_LOG
      sendRequest = 1;
    }
    else if ((KW1281_ASCII == frame.title) && (cnt <= 8))
    {
      frame.data[frame.len-3] = '\n';
      frame.data[frame.len-2] = 0;

      if (f_puts(frame.data, log_file) < 0)
      {
        errorCode = 2;
        break;
      }
      cnt = 0;
    }
    else if ((KW1281_NO_ACK == frame.title) && (cnt > 8))
    {
      p += sprintf(p, "n/a;n/a;n/a;n/a;");
      sendRequest = 1;
    } 
    else if (cnt > 8) // other response 
    {
      p += sprintf(p, "0x%02x;;;;", frame.title);
      sendRequest = 1;
    #ifdef DEBUG_LOG
      if (kw1281Debug)
      {
        buf_p += Byte2HexStr(buf_p, frame.title);
        *buf_p++ = ',';
        buf_p += Byte2HexStr(buf_p, frame.len);
        *buf_p++ = ',';
        for (i=0; i<frame.len; i++)
        {
          buf_p += Byte2HexStr(buf_p, frame.data[i]);
        }
        *buf_p++ = ';';
      }
    #endif //DEBUG_LOG
    }

    //last group in current line received, save it to log file
    if ((cnt > 8) && (requestedGroup == config[config[0]]))
    {
      //save units header to file
      if (units_buf_p)
      {
        *units_buf_p++ = '\n';
        *units_buf_p = 0;
        if (f_puts(units_buf, log_file) < 0)
        {
          errorCode = 2;
          break;
        }
        units_buf_p = 0;
      }

      *p++ = '\n';
      *p = 0;
      if (f_puts(currentLogLine, log_file) < 0)
      {
        errorCode = 2;
        break;
      }
      p = currentLogLine;
      p += sprintf(p, "%d.%02d;", timeSec, time10MSec);
      
    #ifdef DEBUG_LOG
      if (kw1281Debug)
      {
        *buf_p++ = '\n';
        *buf_p = 0;
        if (f_puts(buf, log_file) < 0)
        {
          errorCode = 2;
          break;
        }
        buf_p = buf;
        buf_p += sprintf(buf_p, "%d.%02d;", timeSec, time10MSec);
      }
    #endif // DEBUG_LOG
    }

    
    for (i=0; i < kw1281_intermessage_delay; i++)
    {
      delay(4000); //1ms
    }
    
    if (sendRequest)
    {
      frame.len = 4;
      frame.title = KW1281_GROUP_REQ;
      KW1281BlockCounter++; // inkrementacja lokalnie przechowywanej wartosci Block Counter
      frame.cnt = KW1281BlockCounter;
      KW1281data[0] = groupNumber;
      requestedGroup = groupNumber;
      
    #ifdef DEBUG_LOG
      if (kw1281Debug)
      {
        memmove(&debug_frame, &frame, sizeof(frame));
      }
    #endif // DEBUG_LOG
      result = KW1281SendBlock(&frame); //wyslanie bloku do ECU 
      if (result) 
      {
        //trans_GROUP_REQ_block
        errorCode = 1;
      #ifdef DEBUG_LOG        
        stage = 2;
      #endif
        break;
      }
      
      //next group
      groupPos = (groupPos < config[0])?groupPos+1:1;
      groupNumber = config[groupPos];
    }
    else
    {
      frame.len = 3;
      frame.title = KW1281_ACK;
      KW1281BlockCounter++; // inkrementacja lokalnie przechowywanej wartosci Block Counter
      frame.cnt = KW1281BlockCounter;
      KW1281data[0] = 0;
      
    #ifdef DEBUG_LOG
      if (kw1281Debug)
      {
        memmove(&debug_frame, &frame, sizeof(frame));
      }
    #endif // DEBUG_LOG
      result = KW1281SendBlock(&frame); //wyslanie bloku do ECU 
      if (result) 
      {
        //error trans_ACK_block
        errorCode = 1;
      #ifdef DEBUG_LOG
        stage = 3;
      #endif
        break;
      }  
    }
  }

#ifdef DEBUG_LOG
  if ((kw1281Debug) && (1 == errorCode))
  {
    buf_p = buf;
    buf_p += sprintf(buf_p, "\n%d.%02d; communication error, stage=%d, result=0x%02x\n", timeSec, time10MSec, stage, result);
    result = f_puts(buf, log_file);

    buf_p = buf;
    buf_p += sprintf(buf_p, "frame.len=0x%02x, frame.title=0x%02x, frame.cnt=0x%02x KW1281data[0]= 0x%02x, KW1281BlockCounter=0x%02x\n", frame.len, frame.title, frame.cnt, KW1281data[0], KW1281BlockCounter);
    result = f_puts(buf, log_file);

    buf_p = buf;
    buf_p += sprintf(buf_p, "last transmitted frame: debug_frame.len=0x%02x, debug_frame.title=0x%02x, debug_frame.cnt=0x%02x\n", debug_frame.len, debug_frame.title, debug_frame.cnt);
    result = f_puts(buf, log_file);
  }
#endif // DEBUG_LOG

  return errorCode;
}


#ifdef DEBUG_LOG
void DebugKWP2000Frame(FIL *log_file, u8 * kwp)
{
  u8 debugString[256];
  u8 *p = debugString;
  u8 i;

  p += sprintf(p, "KWP2000 msg, len=%d, ", kwp[0]);
  for (i=1; i<=kwp[0]; i++)
  {
    p += sprintf(p, "%02x ", kwp[i]);
  }
  p += sprintf(p, "\n");
  f_puts(debugString, log_file);
}
#endif // DEBUG_LOG


// return 0 - terminated by user
// return 1 - vwtp error
// return 2 - filesystem error

u8 vwtp(u8 *config, FIL *log_file, u8 vwtpDebug) 
{
  u8 groupNumber = config[1];
  u8 groupPos = 1;
  u8 currentLogLine[150];
  u8 *currentLogLine_p = currentLogLine;
  u8 errorCode;
  u8 kwp[128];
  VWTP_Result_t result;

  
  u8 units_buf[150];
  u8 *units_buf_p = units_buf;
  
  u8 file_header[120];
  u8 *file_header_p = file_header;
  u8 i, j;
  
  CAN_FlushReceiveFifo();

#ifdef DEBUG_LOG_CAN_FRAME
  if (vwtpDebug)
  {
    CAN_Debug(1, log_file);
  }
  else
  {
    CAN_Debug(0, log_file);
  }
#endif // DEBUG_LOG

  // 1. Connect with ECU
  result = VWTP_Connect();
  if (result != VWTP_OK)
  {
    return 11;
  }

  // 2. Start Diagnostic Session
  kwp[0] = 0;
  result =  VWTP_KWP2000Message(0x10, 0x89, kwp);
  if (result != VWTP_OK)
  {
    return 12;
  }
  
#ifdef DEBUG_LOG
  if (vwtpDebug)
  {
    DebugKWP2000Frame(log_file, kwp);
  }
#endif

  if ( (kwp[0] != 4) || (kwp[2] != 2) || (kwp[3] != 0x50) || (kwp[4] != 0x89) )
  {
    return 13;
  }

  // 3. ACK
  result = VWTP_ACK();
  if (result != VWTP_OK)
  {
    return 14;
  }
  
  // 4. ReadEcuId
  kwp[0] = 0;
  result =  VWTP_KWP2000Message(0x1A, 0x9B, kwp);
  if (result != VWTP_OK)
  {
    return 15;
  }
  
#ifdef DEBUG_LOG
  if (vwtpDebug)
  {
    DebugKWP2000Frame(log_file, kwp);
  }
#endif

  if ((kwp[3] == 0x5A) && (kwp[4] == 0x9B) )
  {
    memmove(currentLogLine, kwp+5, 11);
    currentLogLine[11] = '\n';
    currentLogLine[12] = 0;
    if (f_puts(currentLogLine, log_file) < 0)
    {
      return 2; //filesystem error
    }
  }

  // 5. ACK
  result = VWTP_ACK();
  if (result != VWTP_OK)
  {
    return 16;
  }
  
  // 6. Start Routine By Local Identifier
  kwp[0] = 2;
  kwp[1] = 0;
  kwp[2] = 0;
  result =  VWTP_KWP2000Message(0x31, 0xB8, kwp);
  if (result != VWTP_OK)
  {
    return 17;
  }
  
#ifdef DEBUG_LOG
  if (vwtpDebug)
  {
    DebugKWP2000Frame(log_file, kwp);
  }
#endif

  if ( (kwp[3] != 0x71) || (kwp[4] != 0xB8) )
  {
    return 18;
  }

  // 7. ACK
  result = VWTP_ACK();
  if (result != VWTP_OK)
  {
    return 19;
  }
  
  // create file header
  file_header_p = file_header;
  file_header_p += sprintf(file_header_p, "time;");
  for (i=1; i <= config[0]; i++)
  {
    for (j=1; j <= 4; j++)
    {
      file_header_p += sprintf(file_header_p,"%d %d;", config[i], j);
    }
  }
  file_header_p += sprintf(file_header_p, "\n");
  
  currentLogLine_p += sprintf(currentLogLine_p, "%d.%02d;", timeSec, time10MSec);

  units_buf_p += sprintf(units_buf_p, "s;");

  while (1)
  {
    kwp[0] = 0;
    result =  VWTP_KWP2000Message(0x21, groupNumber, kwp);
    if (result != VWTP_OK)
    {
      errorCode = 20;
      break;
    }
  #ifdef DEBUG_LOG
    if (vwtpDebug)
    {
      DebugKWP2000Frame(log_file, kwp);
    }
  #endif

    
    if ( (kwp[3] == 0x61) && (kwp[4] == groupNumber) ) //group response
    {
      currentLogLine_p += DecodeFrame(currentLogLine_p, &kwp[5]);
      
      if (units_buf_p)
      {
        units_buf_p += DecodeUnits(units_buf_p, &kwp[5]);
      }
    }
    else if ( (kwp[3] == 0x7f) && (kwp[4] == 0x21) ) //group response error
    {
      currentLogLine_p += sprintf(currentLogLine_p, ";;;;;");
      
      if (units_buf_p)
      {
        units_buf_p += sprintf(units_buf_p, ";;;;;");
      }
    }
    else
    {
      errorCode = 21;
      break;
    }
  
    if (buttonState)
    {
      errorCode = 0; //logging terminated by user
      break;
    }

    //last group in current line received, save it to log file
    if (groupPos == config[0])
    {
      //save units header to file
      if (units_buf_p)
      {
        //write header to log file
        if ((f_puts("-------------\n\n", log_file) < 0) ||
            (f_puts(file_header, log_file) < 0))
        {
          errorCode = 2;
          break;
        }
        
        *units_buf_p++ = '\n';
        *units_buf_p = 0;
        if (f_puts(units_buf, log_file) < 0)
        {
          errorCode = 2;
          break;
        }
        units_buf_p = 0;
      }
      
      *currentLogLine_p++ = '\n';
      *currentLogLine_p = 0;
      if (f_puts(currentLogLine, log_file) < 0)
      {
        errorCode = 2;
        break;
      }
      currentLogLine_p = currentLogLine;
      currentLogLine_p += sprintf(currentLogLine_p, "%d.%02d;", timeSec, time10MSec);
      
      groupPos = 1;
      groupNumber = config[groupPos];
    }
    else
    {
      //next group
      groupPos = (groupPos < config[0])?groupPos+1:1;
      groupNumber = config[groupPos];
    }

    result = VWTP_ACK();
    if (result != VWTP_OK)
    {
      errorCode = 22;
      break;
    }
  }
  
  if (0 == errorCode)
  {
    result = VWTP_Disconnect();
  }

  return errorCode;
}

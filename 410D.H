        /**********************************************************
        /           410 series flow computer header file          /
        /           Update : 1999/10/11                           /
        **********************************************************/

#include ".\iord2.h"
#include "c:\icc8051\include\stdio.h"
#include "c:\icc8051\include\stdlib.h"
#include "c:\icc8051\include\string.h"
#include "c:\icc8051\include\math.h"

#define     RTC                 (   ( char * ) 0x014000 )
#define     NVRAM               (   ( char * ) 0x01400e )
#define		NVCHAR				(   ( char * ) 0x018000 )
#define		NVLONG				(   ( long * ) 0x018200 )

/**********************************************************
*           Flowmeter Signal Selection                    *
**********************************************************/

#define     Meter1              PULSE_FLOW
#define     Meter2              PULSE_FLOW
#define     PULSE_FLOW          0
#define     ANALOG_FLOW         1

#define     DisMode             LCD_DISPLAY
#define     LCD_DISPLAY         0
#define     VFD_DISPLAY         1

#define     Compensation        NONE
#define     NONE                0
#define     LIQUID              1

/**********************************************************
*           Variables in the Nonvolatile Memory           *
**********************************************************/

#define     Total_DP2           NVRAM[0]
#define     Rate_DP2            NVRAM[1]
#define     Correct2            NVRAM[2]
#define     TimeBase2           NVRAM[3]
#define     Total_unit1         NVRAM[4]    /* overall total unit */
#define     Signal              NVRAM[5]    /* comm.signal type */
#define     Baud                NVRAM[6]    /* comm. baud_rate */
#define     CommType            NVRAM[7]    /* logging type-0:pc, 1:printer */
#define     PrintMode           NVRAM[8]    /* 0: key, 1: time interval */
#define     Print_unit          NVRAM[9]    /* 0: none, 1: print its unit */
#define     Interval            NVRAM[10]   /* print interval 0-6 */
#define     ResetMode           NVRAM[11]   /* reset method resetkey, print,24hour */
#define     DateFormat          NVRAM[12]   /* date format, USA,EUP,KOR */
#define     ID_A                NVRAM[13]
#define     ID_B                NVRAM[14]
#define     TimeBase1           NVRAM[15]
#define     Total_DP1           NVRAM[16]
#define     Rate_DP1            NVRAM[17]
#define     AutoReturn          NVRAM[18]   /* 0:disable, 1:enable */
#define     Correct1            NVRAM[19]
#define     Seq                 NVRAM[20]   /* Relay1,2 operating point */
#define     Filter1             NVRAM[21]   /* Digital Filtering constant 1-99 */
#define     Warning             NVRAM[22]   /* Warning Enable */
#define     VFD_mode            NVRAM[23]   /* brightness */
#define     CV_A1               NVRAM[24]   /* Cutvalue */
#define     CV_B1               NVRAM[25]
#define     DB_A                NVRAM[26]   /* DeadBand */
#define     DB_B                NVRAM[27]


#define     Char                unsigned char
#define     Int                 unsigned int
#define     Long                unsigned long
#define     MINUS               0
#define     PLUS                1

#define     Basic               ( ext_key_load() & 0xf0 ) == 0xf0
#define     Basic_A             ( ext_key_load() & 0xf0 ) == 0xe0
#define     Basic_R             ( ext_key_load() & 0xf0 ) == 0xd0
#define     Basic_C             ( ext_key_load() & 0xf0 ) == 0xb0
#define     Basic_AC            ( ext_key_load() & 0xf0 ) == 0xa0
#define     Basic_RC            ( ext_key_load() & 0xf0 ) == 0x90

#define     Basic_NR            ( ext_key_load() & 0xa0 ) == 0xa0

#define     FLOW1               0x00    /* SEL2_OFF  SEL1_OFF  SEL0_OFF  */
#define     FLOW2               0x01    /* SEL2_OFF  SEL1_OFF  SEL0_ON  */
#define     TEMP1               0x02    /* SEL2_OFF  SEL1_ON   SEL0_OFF  */
#define     TEMP2               0x03    /* SEL2_OFF  SEL1_ON   SEL0_ON   */
#define     RTD1                0x04    /* SEL2_ON   SEL1_OFF  SEL0_OFF  */
#define     RTD2                0x05    /* SEL2_ON   SEL1_OFF  SEL0_ON   */
#define     PRESS1              0x06    /* SEL2_ON   SEL1_ON   SEL0_OFF  */
#define     PRESS2              0x07    /* SEL2_ON   SEL1_ON   SEL0_ON   */
#define     SEL_CLR             0x08

#define     MIN                 819
#define     MAX                 4094
#define     RANGE               ( MAX - MIN )

#define     Alarm1              Option[0]
#define     Alarm2              Option[1]
#define     ZeroAlarm           Option[2]
#define     SpanAlarm           Option[3]
#define     Factor1             Option[4]
#define     S_Factor1           Option[4]
#define     Z_Factor1           Option[5]
#define     Factor2             Option[6]
#define     S_Factor2           Option[6]
#define     Z_Factor2           Option[7]

#define     Temp1_4             Value[0]
#define     Temp1_20            Value[1]
#define     Temp1_offset        Value[2]

#if     Compensation == NONE
#define     Press_atm           Value[3]
#define     Press1_4            Value[4]
#define     Press1_20           Value[5]

#elif   Compensation == LIQUID
#define     Temp2_4             Value[3]
#define     Temp2_20            Value[4]
#define     Temp2_offset        Value[5]
#endif


#define     ErrPulse_H          EOUT = 0
#define     ErrPulse_L          EOUT = 1
#define     OutPulse_H          POUT = 0
#define     OutPulse_L          POUT = 1

#define     RLY1_ON             RLY1 = 0
#define     RLY1_OFF            RLY1 = 1
#define     RLY2_ON             RLY2 = 0
#define     RLY2_OFF            RLY2 = 1

#define     Buzz_ON             BUZZ = 0
#define     Buzz_OFF            BUZZ = 1

#define     SEL0_ON             SEL0 = 1
#define     SEL0_OFF            SEL0 = 0
#define     SEL1_ON             SEL1 = 1
#define     SEL1_OFF            SEL1 = 0
#define     SEL2_ON             SEL2 = 1
#define     SEL2_OFF            SEL2 = 0

#define     ADC_ON              ADC = 1
#define     ADC_OFF             ADC = 0



/**********************************************************
*           Description of Key in Front Display           *
**********************************************************/

#define     kMode               key_load() == 0xf0
#define     kShift              key_load() == 0xe8
#define     kUp                 key_load() == 0xd8
#define     kDown               key_load() == 0xb8
#define     kEnter              key_load() == 0x78
#define     kSetting            key_load() == 0xa8

#define     N_kMode             key_load() != 0xf0
#define     N_kShift            key_load() != 0xe8
#define     N_kUp               key_load() != 0xd8
#define     N_kDown             key_load() != 0xb8
#define     N_kEnter            key_load() != 0x78
#define     N_kSetting          key_load() != 0xa8

#define     Mode                0xf0
#define     Shift               0xe8
#define     Up                  0xd8
#define     Down                0xb8
#define     Enter               0x78
#define     Setting             0xa8

/**********************************************************
*           Description of Remote Key Operation           *
**********************************************************/


#define     NR_Interlock2       ext_key_load1() == 0x0b
#define     NR_Interlock1       ext_key_load1() == 0x07

#define     R_Interlock         ext_key_load1() != 0x0b
#define     R_Reset             ext_key_load1() != 0x07

#define     Interlock2          0x0b
#define     Interlock1          0x07



/**********************************************************
*           Description of Remote Key Operation           *
**********************************************************/
/*
#define     NR_Interlock2       ( RDPort_B & 0x20 )    * interlock terminal *
#define     NR_Interlock1       ( RDPort_B & 0x40 )    * Reset terminal *


#define     R_Interlock2        ( RDPort_B | 0xdf )
#define     R_Interlock1        ( RDPort_B | 0xbf ) 


#define     Interlock2          0x20
#define     Interlock1          0x40 
*/
/**********************************************************
*           Description of RTC Address                    *
**********************************************************/
#define     rCENTURY            50
#define     rYEAR               9
#define     rMONTH              8
#define     rDAY                7
#define     rHOUR               4
#define     rMIN                2
#define     rSEC                0

/**********************************************************
*           Description of RTC Address                    *
**********************************************************/

#define     CLK                 P5.0
#define     DI                  P5.1
#define     DO                  P1.7  /* switch4  imsi use */
#define     EEP                 P3.5
#define     DIS0                P5.3
#define     DIS1                P5.4
#define     SWC                 P5.5
#define     SWD                 P5.2
#define     TXE                 P1.0

#define     EOUT                P4.1
#define     POUT                P4.0
#define     RLY1                P1.4
#define     RLY2                P1.5
#define     BUZZ                P4.7
#define     ADC                 P5.6
#define     DAC                 P5.7
#define     SEL0                P4.2
#define     SEL1                P4.3
#define     SEL2                P4.4

#define     EXSWC               P4.5
#define     EXSWD               P4.6
#define     Pulse_W             MS10
#define     MS10                0
#define     MS100               1
#define     MS200               2


#pragma memory = no_init

Int         Zero;
Int         Span;
Long        Option[8];

Long        GrossTotal1;
Long        GrossAccTotal1;
float       GrossRemain1;
float       GrossAccRemain1;

Long        GrossTotal2;
Long        GrossAccTotal2;
float       GrossRemain2;
float       GrossAccRemain2;

#pragma memory = xdata


Char        ID[2];
Int         DeadBand;           /* Relay hysteresis */
Char        SSound = 0;
Char        FSound = 1;
Long        Total;
float       Factor;             /* Meter Factor = 1 / K-Factor */


Int         Rate1[100];

Int         Rate2[100];



#pragma memory = idata

Char        DisCount;           /* 10mSec Interrupt Counter */
Char        DisEn = 0;          /* Display Enable Flag 260mSec */
Char        Dis[10];            /* Display Code Ascii Table */

Char        FilterCounter1;     /* Rate Filter Counter for CH#1 */
Char        FilterCounter2;     /* Rate Filter Counter for CH#2 */

Char        DisTimer = 0;       /* Display timer for return */

Long        RATE;               /* Rate Counter */

float       ANL;                /* analog input Value */
float       Rate_Fac1;          /* Meter Factor * TimeBase for CH#1 */
float       Rate_Fac2;          /* Meter Factor * TimeBase for CH#2 */
float       RateHalf1;
float       RateHalf2;
float       Total_Fac1;
float       Total_Fac2;


#pragma memory = data

Char        interlock1_flag = 0;
Char        interlock2_flag = 0;

char        Data;
Char        Pulse = 1;
Char        PulseFlag = 1;
Char        Rly1_flag=0;
Char        Rly2_flag=0;
Char        error = 0;
Long        OldTotal;
Long        DisRate1;
Long        DisRate2;
long        DisNetRate;
Int         TempTotal = 0;
float       Analog;             /* analog output range */


#if     Meter1 == PULSE_FLOW
Int         PulTotal1 = 0;      /* Interrupt Counter1 */
Int         PreTotal1 = 0;
Int         Offset1 = 0;
#endif

#if     Meter2 == PULSE_FLOW
Int         PulTotal2 = 0;      /* Interrupt Counter2 */
Int         PreTotal2 = 0;
Int         Offset2 = 0;
#endif

#pragma memory = default

/**********************************************************
*                     Common   Function                   *
**********************************************************/

Int Ipow( Char p, Char dp )
{
    Char i;
    Int k = 1;

    for( i=0; i<dp; i++ )                 k *= p;
    return k;
}


void delay( Int k )
{
    Int i;

    for( i=0 ; i<k ; i++ );

}


void beep(void)
{
    Pulse = 0;          Buzz_ON;            Pulse = 1;
    delay( 7500 );
    Pulse = 0;          Buzz_OFF;           Pulse = 1;
}


Char key_load(void)
{
    Char value = 0, i;

    SWC = 0;                                /* key load */
    CLK = 0;                                CLK = 1;
    SWC = 1;

    value = SWD;

    for( i=0 ; i<7 ; i++ )
    {
        value <<= 1;
        CLK = 0;                            CLK = 1;
        if( SWD )                           value |= 1;
    }

    CLK = 0;                                value &= 0xf8;

    return value;
}
Char ext_key_load(void)
{
    Char value = 0, i;

    EXSWC = 0;                                /* key load */
    EXSWC = 0;
	CLK = 0;
	CLK = 0;
	CLK = 1;
	CLK = 1;
    EXSWC = 1;
	EXSWC = 1;

    value = EXSWD;
	value = EXSWD;

    for( i=0 ; i<7 ; i++ )
    {
        value <<= 1;
        CLK = 0;                            
		CLK = 0;
        CLK = 1;                            
		CLK = 1;

        if( EXSWD )                         value |= 1;
    }

    CLK = 0;   
	CLK = 0;
	value &= 0xf0;

    return value;
}

Char ext_key_load1(void)
{
    Char value = 0, i;

    EXSWC = 0;                                /* key load */
    EXSWC = 0;
	CLK = 0;
	CLK = 0;
	CLK = 1;
	CLK = 1;
    EXSWC = 1;
	EXSWC = 1;

    value = EXSWD;
	value = EXSWD;	

    for( i=0 ; i<7 ; i++ )
    {
        value <<= 1;
        CLK = 0;                            
		CLK = 0;
        CLK = 1;                            
		CLK = 1;

        if( EXSWD )                         value |= 1;
    }

    CLK = 0;   
	CLK = 0;
	value &= 0x0f;

    return value;
}
void key_ready( Char value )
{
    delay( 1500 );                          while( key_load() == value );
    delay( 2500 );
}

void remote_ready( Char value )
{
    delay( 1500 );                          
	while( ext_key_load() == value );
    delay( 2500 );
}

Int ad_converter( Char ch )
{
#pragma memory = xdata
    Int result = 0;
    Char i;
#pragma memory = default

    if (ch == 0)       { SEL2 = 0;   SEL1 = 0;   SEL0 = 0;  }   /* FLOW1  SELECT */
	else if (ch == 1)  { SEL2 = 0;   SEL1 = 0;   SEL0 = 1;  }   /* FLOW2  SELECT */
	else if (ch == 2)  { SEL2 = 0;   SEL1 = 1;   SEL0 = 0;  }   /* TEMP1  SELECT */
    else if (ch == 3)  { SEL2 = 0;   SEL1 = 1;   SEL0 = 1;  }   /* TEMP2  SELECT */
    else if (ch == 4)  { SEL2 = 1;   SEL1 = 0;   SEL0 = 0;  }   /* RTD1   SELECT */
    else if (ch == 5)  { SEL2 = 1;   SEL1 = 0;   SEL0 = 1;  }   /* RTD2   SELECT */
    else if (ch == 6)  { SEL2 = 1;   SEL1 = 1;   SEL0 = 0;  }   /* PRESS1 SELECT */
    else if (ch == 7)  { SEL2 = 1;   SEL1 = 1;   SEL0 = 1;  }   /* PRESS2 SELECT */
	delay( 3000 );

    ADC_OFF;
    for( i=0; i<4 ; i++ )
    {
        CLK = 0;                            delay( 10 );
        if( i == 2 )                        DI = 0;
        else                                DI = 1;
        CLK = 1;                            delay( 10 );
    }
    CLK = 0;                                delay( 10 );
    CLK = 1;                                delay( 10 );

    for( i=0; i<12 ; i++ )
    {
        CLK = 0;                            delay( 10 );
        if( DO )                            result |= 1;
        CLK = 1;                            delay( 10 );
        if( i < 11 )                        result <<= 1;
        else                                ADC_ON;
    }
    return result;
}

        /***************************************************************
        *                        Analog Output                         *
        ***************************************************************/
void DA( Int value )
{
    Char i;

    DAC = 1;
    for( i=0 ; i<4 ; i++ )                  value <<= 1;
    for( i=0 ; i<12 ; i++ )
    {
        CLK = 0;

        if( value & 0x8000 )                DI = 1;
        else                                DI = 0;

        value <<= 1;                        CLK = 1;
    }
    DAC = 0;
    delay(10);
    DAC = 1;
}

void itoa_converter( long value, Char digit, Char dp  )
{
        /*----------------------------------------------*/
        /*       count from left, 1,2,3,4,5,6,7...      */
        /*  if dp > 0, then decimal point is exist      */
        /*           and dot position is at dp.         */
        /*  value -> int or long                        */
        /*  digit -> digit with number (0~9)            */
        /*----------------------------------------------*/
    Char    t, i, j=0, dis[10];
    long    a = 1;

    for( i=0 ; i<digit-1 ; i++ )            a *= 10;

    value %= a*10;

    for( i=0; i<9 ; i++ )                   dis[i] = ' ';
    if( dp )                                j = 1;

    for( i=0 ; i<digit+j ; i++ )
    {
        if( j && i == dp-1 )                dis[i] = '.';
        else
        {
            t = value / a;                  dis[i] = t+'0';
            value %= a;                     a /= 10;
        }
    }

    for( i=0; i<10 ; i++ )                  Dis[i] = dis[i];
}


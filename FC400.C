/************************************************************************
*                                                                       *
*           Flow Controller( FC4XX ) Series                             *
*                                                                       *
*       Oscillator  : 18.4320MHz                                        *
*       CPU         : DALLAS 80C320                                     *
*       RTC         : DALLAS 12C887( Y2K Compatible )                   *
*       Display     : 410 - 20 Charactor * 2 line LCD with EL Backlight *
*                     420 - 20 Charactor * 2 line VFD                   *
*                                                                       *
*       Company     : Flos Korea Co.                                    *
*       Date        : 2000. 09. 04.                                     *
*       Version     : Analog and Frequency Common type                  *
*              4.00 : Density B/D 삽입                  2012.11.20      *
*              4.10 : 강화 테스트용                     2017.01.04      *
*                                                                       *
************************************************************************/

#include ".\410d.h"

#if     DisMode == VFD_DISPLAY
#define     MODEL                   "      FC-420D       "
#elif   DisMode == LCD_DISPLAY
#define     MODEL                   "      FC-410D       "
#endif

#define     VERSION                 "     REV-4.10-S     "

/**********************************************************
*           Variables in the Nonvolatile Memory           *
**********************************************************/
#define     Filter2             NVRAM[28]
#define     CV_A2               NVRAM[29]
#define     CV_B2               NVRAM[30]
#define 	Total_unit2			NVRAM[31]

#pragma memory = no_init




#pragma memory = xdata

Char		F_err1 = 0; 		/* Flow Signal flag for CH#1 */
Char		F_err2 = 0; 		/* Flow Signal flag for CH#2 */
Char        Check_pulse = 0;
/*
float       NetTotalFac = 0;
float       NetRateFac = 0;
*/


#pragma memory = code

Char *Totaldis[8] = { " ltr",  " m",  " ft", " in",
                      " gal",  " kg",  " ton", " " };

                                            /* 0x02 */

Char *prnunit[12] = { "   ltr","    ml","    Ml","    m3","   ft3","   in3",
                      "   gal","  Mgal","barrel","    kg","   ton","      " };

Char *Rateunit[4] = {  "/s", "/m", "/h", "/d" };


#pragma memory = default

#include ".\lcd410.c"
#include ".\serial.c"


void set_ini( void )
{
#pragma memory = xdata

    float           constant = 0;
    signed char     i = 0;
    Char            t = 0;

#pragma memory = default

    /**********************************************************
    *         Initial Parameter for Input Channel #1          *
    **********************************************************/
#if     Meter1 == PULSE_FLOW

    if( !Factor1 )                          Factor1 = 10000;

    Factor = 1. / ( 0.0001 * Factor1 );

    Total_Fac1 = Ipow( 10, Total_DP1 ) * Factor;

    if( TimeBase1 < 3 )                     Rate_Fac1 = Ipow( 60, TimeBase1 );
    else                                    Rate_Fac1 = 24 * 3600;
    Rate_Fac1 *= Ipow( 10, Rate_DP1 ) * Factor;


#endif

    RateHalf1 = 1. / ( Ipow( 10, Rate_DP1 ) * 2. );

    /**********************************************************
    *         Initial Parameter for Input Channel #2          *
    **********************************************************/
#if     Meter2 == PULSE_FLOW

    if( !Factor2 )                          Factor2 = 10000;

    Factor = 1. / ( 0.0001 * Factor2 );

    Total_Fac2 = Ipow( 10, Total_DP2 ) * Factor;

    if( TimeBase2 < 3 )                     Rate_Fac2 = Ipow( 60, TimeBase2 );
    else                                    Rate_Fac2 = 24 * 3600;
    Rate_Fac2 *= Ipow( 10, Rate_DP2 ) * Factor;


#endif

    RateHalf2 = 1. / ( Ipow( 10, Rate_DP2 ) * 2. );

    /************************************************
    *               Rate Array Clear                *
    ************************************************/
    for( i=0 ; i<Filter1 ; i++ )            Rate1[i] = 0;
    for( t=0 ; t<Filter2 ; t++ )            Rate2[t] = 0;

    /**********************************************************
    *              Initial Analog Output Factor               *
    **********************************************************/
	/*
    Analog = Span - Zero;                   
	Analog /= SpanAlarm - ZeroAlarm;
	*/

    /**********************************************************
    *     Net-Total & Net-Rate Calculation Factor initial     *
    **********************************************************/
	/*
    i = Total_DP1 - Total_DP2;

    if( i >= 0 )                  NetTotalFac = Ipow( 10, i );
    else
    {
        i *= -1;                            
		NetTotalFac = ( 1. / Ipow( 10, i ) );
    }

    i = Rate_DP1 - Rate_DP2;

    if( i >= 0 )                  NetRateFac = Ipow( 10, i );
    else
    {
        i *= -1;                            
		NetRateFac = ( 1. / Ipow( 10, i ) );
    }
	*/
}



void ResetValue( Char i )
{
#pragma memory = xdata

    Long    total = 0;

#pragma memory = default

    /***************************************************************
    *                   For Channel#1 Total Clear                  *
    ***************************************************************/
    if( i < 2 )
    {
#if     Meter1 == PULSE_FLOW

        PulTotal1 = 0;                      
		PreTotal1 = 0;
        Offset1 = 0;

#endif
    }
    /***************************************************************
    *                   For Channel#2 Total Clear                  *
    ***************************************************************/
    else
    {
#if     Meter2 == PULSE_FLOW

        PulTotal2 = 0;                      
		PreTotal2 = 0;
        Offset2 = 0;

#endif
    }

    /***************************************************************
    *        Accumulated Total Value Reset for Channel#1           *
    ***************************************************************/
    if( !i )
    {
        GrossAccTotal1 = 0;                 
		GrossAccRemain1 = 0;
     
    }
    /***************************************************************
    *               Total Value Reset for Channel#1                *
    ***************************************************************/
    else if( i == 1 )
    {
        GrossAccTotal1 += GrossTotal1;      GrossAccRemain1 += GrossRemain1;
        total = GrossAccRemain1;            GrossAccRemain1 -= total;
        GrossAccTotal1 += total;            GrossAccTotal1 %= 10000000;

        GrossTotal1 = 0;                    
		GrossRemain1 = 0;
     
    }
    /***************************************************************
    *        Accumulated Total Value Reset for Channel#2           *
    ***************************************************************/
    else if( i == 2 )
    {
        GrossAccTotal2 = 0;                 
		GrossAccRemain2 = 0;
    }
    /***************************************************************
    *               Total Value Reset for Channel#2                *
    ***************************************************************/
    else if( i == 3 )
    {
        GrossAccTotal2 += GrossTotal2;      GrossAccRemain2 += GrossRemain2;
        total = GrossAccRemain2;            GrossAccRemain2 -= total;
        GrossAccTotal2 += total;            GrossAccTotal2 %= 10000000;

        GrossTotal2 = 0;                    
		GrossRemain2 = 0;
    }
    OldTotal = 0;
}    


/*
void Printing( void )
{
    SerialPuts( "\n\rUnit ID                             " );

    SerialPutc( ID[0] );                    
	SerialPutc( ID[1] );

    SerialPuts( "\n\rDate                          " );      
    SendDate( DateFormat );
    SerialPuts( "\n\rTime                          " );      
    SendTime();                           
     
    SerialPuts( "\n\n\rRate            " );
    if( Total_unit == 7 || !Print_unit )
        SerialPuts( "        " );
    else
    {
        SerialPuts( *(prnunit + Total_unit) );
        SerialPuts( *(Rateunit + TimeBase1) );
    }

    SerialPuts( "      " );
    IntAscii( Rate_DP1, DisRate1, PLUS, 1 );
    
    SerialPuts( "\n\rTotal           " );
    if( Total_unit == 7 || !Print_unit )
        SerialPuts( "      " );
    else
        SerialPuts( *(prnunit + Total_unit) );
    SerialPuts( "         " );
    IntAscii( Total_DP1, GrossTotal1, PLUS, 1 );
    
    SerialPuts( "\n\rAcc. Total      " );
    if( Total_unit == 7 || !Print_unit )
        SerialPuts( "      " );
    else
        SerialPuts( *(prnunit + Total_unit) );
    SerialPuts( "         " );
    IntAscii( Total_DP1, GrossTotal1 + GrossAccTotal1, PLUS, 1 );
    SerialPuts( "\n\r\n\n\n" );
}
*/



void communication( void )
{                                  
    while( Buf.Head != Buf.Tail )
    {
        if( Buf.Buff[Buf.Tail] == '\r' )
        {
            Buffer[Pos++] = '\r';

            if( Buffer[0] == 'I' && Buffer[1] == ID[0] && Buffer[2] == ID[1] )
            {
                if( Signal )
                {
                    TXE = 1;
                    if( Signal == 2 )       REN0 = 0;
                }

                SerialPutc( ID[0] );        
				SerialPutc( ID[1] );
                SerialPutc( ' ' );
                     

                if( Buffer[3] == 'R' )
                {               
                    /**************************************************************************
					*                 READ Channel 1  Actual data                             *
					**************************************************************************/
					
                    if( Buffer[4] == '0' && Buffer[5] == '1' )           /* Read CH#1 Flow Rate */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Rate_DP1) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                     

                        IntAscii( Rate_DP1, DisRate1, PLUS, 1, 9 );
					}
                    else if( Buffer[4] == '0' && Buffer[5] == '2' )      /* Read CH#1 Total     */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Total_DP1) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                        IntAscii( Total_DP1, GrossTotal1, PLUS, 1, 9 );
					}
                    else if( Buffer[4] == '0' && Buffer[5] == '3' )      /* Read CH#1 ACC-Total */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Total_DP1) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                        IntAscii( Total_DP1, GrossTotal1 + GrossAccTotal1, PLUS, 1, 9 );
					}
					else if( Buffer[4] == '3' && Buffer[5] == '1' )		/* Read CH#1 Actual data */
					{
						SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
						SerialPutc( ' ' );

						if (!Rate_DP1) SerialPutc( ' ' );
                        IntAscii( Rate_DP1, DisRate1, PLUS, 1, 9 );

						if (!Total_DP1) SerialPutc( ' ' );                 
                        IntAscii( Total_DP1, GrossTotal1, PLUS, 1, 9 );

						if (!Total_DP1) SerialPutc( ' ' );                       
                        IntAscii( Total_DP1, GrossTotal1 + GrossAccTotal1, PLUS, 1, 9 );
					}
					else if( Buffer[4] == '3' && Buffer[5] == '4' )		/* Read CH#1 All data */
					{
						SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
						SerialPutc( ' ' );

						if (!Rate_DP1) SerialPutc( ' ' );
                        IntAscii( Rate_DP1, DisRate1, PLUS, 1, 9 );

						if (!Total_DP1) SerialPutc( ' ' );                 
                        IntAscii( Total_DP1, GrossTotal1, PLUS, 1, 9 );
					}

                    /**************************************************************************
					*                 READ Flow Channel 2  Actual data                        *
					**************************************************************************/

                    else if( Buffer[4] == '1' && Buffer[5] == '1' )      /* Read CH#2 Flow Rate  */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Rate_DP2) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                        IntAscii( Rate_DP2, DisRate2, PLUS, 1, 9 );
					} 
                    else if( Buffer[4] == '1' && Buffer[5] == '2' )      /* Read CH#2 Total */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Total_DP2) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                        IntAscii( Total_DP2, GrossTotal2, PLUS, 1, 9 );
					} 
                    else if( Buffer[4] == '1' && Buffer[5] == '3' )      /* Read CH#2 AccTotal */  
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Total_DP2) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                        IntAscii( Total_DP2, GrossTotal2 + GrossAccTotal2, PLUS, 1, 9 );
					}
					else if( Buffer[4] == '3' && Buffer[5] == '2' )      /* Read CH#2 Actual Data */  
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
						SerialPutc( ' ' );

						if (!Rate_DP2) SerialPutc( ' ' );                        
                        IntAscii( Rate_DP2, DisRate2, PLUS, 1, 9 );

						if (!Total_DP2) SerialPutc( ' ' );                        
                        IntAscii( Total_DP2, GrossTotal2, PLUS, 1, 9 );

						if (!Total_DP2) SerialPutc( ' ' );
                        IntAscii( Total_DP2, GrossTotal2 + GrossAccTotal2, PLUS, 1, 9 );
					}
					else if( Buffer[4] == '3' && Buffer[5] == '5' )      /* Read CH#2 AccTotal */  
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
						SerialPutc( ' ' );

						if (!Rate_DP2) SerialPutc( ' ' );                        
                        IntAscii( Rate_DP2, DisRate2, PLUS, 1, 9 );

						if (!Total_DP2) SerialPutc( ' ' );                        
                        IntAscii( Total_DP2, GrossTotal2, PLUS, 1, 9 );
					}
					else if( Buffer[4] == '3' && Buffer[5] == '3' )      /* Read All Actual Data */  
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
						SerialPutc( ' ' );

						if (!Rate_DP1) SerialPutc( ' ' );
                        IntAscii( Rate_DP1, DisRate1, PLUS, 1, 9 );

						if (!Total_DP1) SerialPutc( ' ' );                 
                        IntAscii( Total_DP1, GrossTotal1, PLUS, 1, 9 );

						if (!Total_DP1) SerialPutc( ' ' );                       
                        IntAscii( Total_DP1, GrossTotal1 + GrossAccTotal1, PLUS, 1, 9 );

						if (!Rate_DP2) SerialPutc( ' ' );                        
                        IntAscii( Rate_DP2, DisRate2, PLUS, 1, 9 );

						if (!Total_DP2) SerialPutc( ' ' );                        
                        IntAscii( Total_DP2, GrossTotal2, PLUS, 1, 9 );

						if (!Total_DP2) SerialPutc( ' ' );
                        IntAscii( Total_DP2, GrossTotal2 + GrossAccTotal2, PLUS, 1, 9 );
					}
					else if( Buffer[4] == '3' && Buffer[5] == '6' )      /* Read CH#2 AccTotal */  
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
						SerialPutc( ' ' );

						if (!Rate_DP1) SerialPutc( ' ' );
                        IntAscii( Rate_DP1, DisRate1, PLUS, 1, 9 );

						if (!Total_DP1) SerialPutc( ' ' );                 
                        IntAscii( Total_DP1, GrossTotal1, PLUS, 1, 9 );

						if (!Rate_DP2) SerialPutc( ' ' );                        
                        IntAscii( Rate_DP2, DisRate2, PLUS, 1, 9 );

						if (!Total_DP2) SerialPutc( ' ' );                        
                        IntAscii( Total_DP2, GrossTotal2, PLUS, 1, 9 );
					}


                    /**************************************************************************
					*                 READ Inter Lock Status                                  *
					**************************************************************************/
                    else if( Buffer[4] == '2' && Buffer[5] == '0' )     /* InterLock status 1 */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        if (!interlock1_flag)  SerialPutc( '0' );
                        else                   SerialPutc( '1' );
					}
                    else if( Buffer[4] == '2' && Buffer[5] == '1' )     /* InterLock status 2 */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        if (!interlock2_flag)  SerialPutc( '0' );
                        else                   SerialPutc( '1' );
					}


                    /**************************************************************************
					*                 READ Data & Time                                        *
					**************************************************************************/

                    else if( Buffer[4] == '2' && Buffer[5] == '8' )     /* Read Date          */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

					
                        SendDate( 0 );

					}
                    else if( Buffer[4] == '2' && Buffer[5] == '9' )     /* Read Time         */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        SendTime();
					}


                    /**************************************************************************
					*                 READ Channel 1  Configure data                          *
					**************************************************************************/

                    else if( Buffer[4] == '0' && Buffer[5] == '4' )      /* Read CH#1 Rate DP */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        SerialPutc( Rate_DP1 + '0' );
					}
                    else if( Buffer[4] == '0' && Buffer[5] == '5' )      /* Read CH#1 Total DP */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        SerialPutc( Total_DP1 + '0' );
					}
                    else if( Buffer[4] == '0' && Buffer[5] == '6' )      /* Read CH#1 Timebase    */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        SerialPutc( TimeBase1 + '0' );
					}
                    else if( Buffer[4] == '0' && Buffer[5] == '7' )      /* Read CH#1 Flow unit */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        SerialPutc( Total_unit1 + '0' );
					}


                    /**************************************************************************
					*                 READ Channel 2  Configure data                          *
					**************************************************************************/

                    else if( Buffer[4] == '1' && Buffer[5] == '4' )      /* Read CH#2  Rate_DP */
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        SerialPutc( Rate_DP2 + '0' );
					}
                    else if( Buffer[4] == '1' && Buffer[5] == '5' )      /* Read CH#2 Total DP */ 
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        SerialPutc( Total_DP2 + '0' );
					}
                    else if( Buffer[4] == '1' && Buffer[5] == '6' )     /* Read CH#2 TimeBase  */ 
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        SerialPutc( TimeBase2 + '0' );
					}
                    else if( Buffer[4] == '1' && Buffer[5] == '7' )     /* Read CH#2 Flow Unit */ 
					{
                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );
                        SerialPutc( ' ' );

                        SerialPutc( Total_unit2 + '0' );
					}
                    /**************************************************************************/


                    else           SerialPuts( "FORMAT ERR" );

                    SerialPuts( "\r\n" );

                }
                else if( Buffer[3] == 'W' )
                {
					/* channel #1   reset  */
                    if( Buffer[4] == '0' && Buffer[5] == '2' && Buffer[6] == '0' && Buffer[7] == '\r' )
                    {        
                        ResetValue( 1 );

                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Total_DP1) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                        IntAscii( Total_DP1, GrossTotal1, PLUS, 1, 9 );
                    
                    }
					else if( Buffer[4] == '0' && Buffer[5] == '3' && Buffer[6] == '0' && Buffer[7] == '\r' )
                    {        
                        ResetValue( 1 );
                        ResetValue( 0 );

                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Total_DP1) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                        IntAscii( Total_DP1, GrossTotal1 + GrossAccTotal1, PLUS, 1, 9 );
                    }
					/* channel #2   reset  */
                    else if( Buffer[4] == '1' && Buffer[5] == '2' && Buffer[6] == '0' && Buffer[7] == '\r' )
                    {
                        ResetValue( 3 );

                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Total_DP2) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                        IntAscii( Total_DP2, GrossTotal2, PLUS, 1, 9 );
                    }
                    else if( Buffer[4] == '1' && Buffer[5] == '3' && Buffer[6] == '0' && Buffer[7] == '\r' )
                    {
                        ResetValue( 3 );
                        ResetValue( 2 );

                        SerialPutc( Buffer[4] );        
				        SerialPutc( Buffer[5] );

						if (!Total_DP2) SerialPutc( ' ' );
                        SerialPutc( ' ' );

                        IntAscii( Total_DP2, GrossTotal2 + GrossAccTotal2, PLUS, 1, 9 );
                    }                       
					
                    else if( Buffer[4] == '2' && Buffer[5] == '8' )                   
					{
                        if( Buffer[8] == '/' && Buffer[11] =='/' && Buffer[14] =='\r' )
                        { 
                            RTC[11] = 0x82;
                            PutRtc( rYEAR, Buffer[6], Buffer[7] );
                            PutRtc( rMONTH, Buffer[9], Buffer[10] );
                            PutRtc( rDAY, Buffer[12], Buffer[13] );
                            RTC[11] = 0x02;

                            SerialPutc( Buffer[4] );        
				            SerialPutc( Buffer[5] );
					
                            SerialPutc( ' ' );

                            SendDate( 0 );
                        }
                        else                SerialPuts( "FORMAT ERR" );     
                    }
                    else if( Buffer[4] == '2' && Buffer[5] == '9' )
                    {
                        if( Buffer[8] == ':' && Buffer[11] ==':' && Buffer[14] =='\r' )
                        { 
                            RTC[11] = 0x82;

                            PutRtc( rHOUR, Buffer[6], Buffer[7] );
                            PutRtc( rMIN, Buffer[9], Buffer[10] );
                            PutRtc( rSEC, Buffer[12], Buffer[13] );
                          
                            RTC[11] = 0x02;
                            SerialPutc( Buffer[4] );        
				            SerialPutc( Buffer[5] );
					
                            SerialPutc( ' ' );

                            SendTime();                            
                        }
                        else    SerialPuts( "FORMAT ERR" );     
                    }   
                    else        SerialPuts( "FORMAT ERR" ); 

                    SerialPuts( "\r\n" );
                }
                else
                {
                    SerialPuts( "FORMAT ERR" ); 
                    SerialPuts( "\r\n" );
                }

                if( Signal )
                {
                    TXE = 0;
                    if( Signal == 2 )       REN0 = 1;
                }
            }
            else if( Buffer[0] == 'I' && Buffer[1] == '0' && Buffer[2] == '0' )
            {
                if( Buffer[3] == 'W' )
                {
                    if( Buffer[4] == '0' && Buffer[5] == '2' && Buffer[6] == '0' && Buffer[7] == '\r' )
                    {        
                        ResetValue( 1 );
                    }
                    else if( Buffer[4] == '0' && Buffer[5] == '3' && Buffer[6] == '0' && Buffer[7] == '\r' )
                    {
						ResetValue( 1 );
                        ResetValue( 0 );
                    }
                    else if( Buffer[4] == '1' && Buffer[5] == '2' && Buffer[6] == '0' && Buffer[7] == '\r' )
                    {      						
                        ResetValue( 3 );
                    }
                    else if( Buffer[4] == '1' && Buffer[5] == '3' && Buffer[6] == '0' && Buffer[7] == '\r' )
                    {
						ResetValue( 3 );
                        ResetValue( 2 );
                    }                       					
                    else if( Buffer[4] == '2' && Buffer[5] == '8' )
                    {
                        if( Buffer[8] == '/' && Buffer[11] =='/' && Buffer[14] =='\r' )
                        { 
                            RTC[11] = 0x82;
                            PutRtc( rYEAR, Buffer[6], Buffer[7] );
                            PutRtc( rMONTH, Buffer[9], Buffer[10] );
                            PutRtc( rDAY, Buffer[12], Buffer[13] );
                            RTC[11] = 0x02;
                        }
                    }
                    else if( Buffer[4] == '2' && Buffer[5] == '9' )
                    {
                        if( Buffer[8] == ':' && Buffer[11] ==':' && Buffer[14] =='\r' )
                        { 
                            RTC[11] = 0x82;

                            PutRtc( rHOUR, Buffer[6], Buffer[7] );
                            PutRtc( rMIN, Buffer[9], Buffer[10] );
                            PutRtc( rSEC, Buffer[12], Buffer[13] );
                            RTC[11] = 0x02;
                        }
                    }   
                }
            }
        }

        Data = GetQueue( &Buf );

        if( Data == -1 )                    return;
        else if( Data == '\r' )
        {
            Buffer[0] = 0x00;               
			Pos = 0;
            return;
        }
        else if( Signal == 2 && Data == '\n' )
        {
            Buf.Tail = 0;                   Buf.Head = 0;
            Buffer[0] = 0x00;               Buffer[1] = 0x00;
            Pos = 0;                        
			return;
        }

        Buffer[ Pos++ ] = Data;

        if( Pos > 20 )
        {
            Pos = 0;                        
			return;
        }
    }
} 



void Parameter_liquid( Char item )
{
#pragma memory = xdata

    Char x=0, first=0, i, d, digit=7, dis[11];

#pragma memory = code

    Char DIS[][21] = {    " TEMPERATURE at 4mA ",           /* 0 */
                          " TEMPERATURE at 20mA",           /* 1 */
                          "     RTD OFFSET     ",           /* 2 */
                          "    CH. CONSTANT    ",           /* 3 */
                          "ATMOSPHERIC PRESSURE",           /* 4 */
                          "  PRESSURE at 4mA   ",           /* 5 */
                          "  PRESSURE at 20mA  ",           /* 6 */
                          "   IDENTIFICATION   ",           /* 7 */
                          " CH#1 FILTER FACTOR ",           /* 8 */
                          " CH#2 FILTER FACTOR " };         /* 9 */

#pragma memory = default

    LCD_pos( 1, 0 );                        LCD_puts( DIS[item] );
    LCD_pos( 2, 0 );                        LCD_puts( BLANK );

    for( i=0 ; i<11 ; i++ )                 dis[i] = ' ';

    switch( item )
    {

        case 7: digit = 2;
                dis[0] = ID[0];             
				dis[1] = ID[1];
                break;

        case 8: digit = 2;                  
		        Filter1 %= 100;
                dis[0] = Filter1/10 + '0';  
				dis[1] = Filter1%10 + '0';
                break;

        case 9: digit = 2;                  
		        Filter2 %= 100;
                dis[0] = Filter2/10 + '0';  
				dis[1] = Filter2%10 + '0';
                break;
    }

    if( item < 7 )                          LCD_pos( 2, 8 );
    else                                    LCD_pos( 2, 9 );

    for( i=0; i<digit ; i++ )               LCD_putc( dis[i] );
    
    LCD_cursorblink( ON );                  
	key_ready( Enter );

    for(;;)
    {
        sync_data( LCDpos );

        if( ( item > 2 ) || x )             d = dis[x] - '0';

        if( kUp )
        {
            beep();
            LCD_cursorblink( OFF );         
			first = 1;
            if( !x && item < 3 )
            {
                if( dis[x] == '+' )         dis[x] = '-';
                else                        dis[x] = '+';
            }
            else
            {
                d++;                        
				d %= 10;
                dis[x] = d +'0';
            }
            LCD_putc( dis[x] );             
			key_ready( Up );
            LCD_cursorblink( ON );
        }
        else if( kDown )
        {
            beep();
            LCD_cursorblink( OFF );         
			first = 1;

            if( !x && item < 3 )
            {
                if( dis[x] == '+' )         dis[x] = '-';
                else                        dis[x] = '+';
            }
            else
            {
                if( !d )                    d = 10;
                d--;                        
				dis[x] = d +'0';
            }
            LCD_putc( dis[x] );             
			key_ready( Down );
            LCD_cursorblink( ON );
        }
        else if( kShift )
        {
            beep();                         
			first = 1;
            LCDpos++;                       
			x++;

            if( dis[x] == '.' )
            {
                LCDpos++;                   
				x++;
            }
            if( x == digit )
            {
                x = 0;
                if( item < 7 )              LCD_pos( 2, 8 );
                else                        LCD_pos( 2, 9 );
            }
            key_ready( Shift );
        }
        else if( kMode )
        {
            beep();
            LCD_cursorblink( OFF );
            if( !first )
            {
                if( item < 3 )              dis[0] = '+';
                else if( item < 10 )
                {
                    dis[0] = '0';
                    if( item > 7 )          dis[1] = '1';
                }

                LCD_putc( dis[0] );

                if( item > 7 )             LCD_putc( dis[1] );
                else
                {
                    for( i=1; i<digit; i++ )
                    {
                        if( dis[i] != '.' )
                            dis[i] = '0';
                        LCD_putc( dis[i] );
                    }
                }
            }
            else
            {
                if( !x && item<3 )          dis[x] = '+';
                else                        dis[x] = '0';
                LCD_putc( dis[x] );
            }
            key_ready( Mode );              
			LCD_cursorblink( ON );
        }
        else if( kEnter )
        {
            beep();                         
			LCD_cursorblink( OFF );

            switch( item )
            {

                case 7: ID[0] = dis[0];     
				        ID[1] = dis[1];

                        ID_A = ID[0];       
						ID_B = ID[1];
                        break;

                case 8: Filter1 = ( dis[0] - '0' ) * 10;
                        Filter1 += dis[1] - '0';
                        if( !Filter1 )      Filter1 = 1;
                        break;

                case 9: Filter2 = ( dis[0] - '0' ) * 10;
                        Filter2 += dis[1] - '0';
                        if( !Filter2 )      Filter2 = 1;
                        break;
            }
            key_ready( Enter );
            set_ini();
            break;

        }
    }
}


void Parameter_flow( Char item )
{
#pragma memory = xdata

    Char    x=0, first=0, i, d, digit, dis[11], error=0;
    long    a;
    float   constant=0;

#pragma memory = code



    Char *Total[8] = { "ltr",  "m",  "ft", "in",
                       "gal",  "kg",  "ton", " " };

                                            /* 0x02 */


    Char SIGNAL[][21] = { "   CH#1 FLOW SPAN   ",           /* 0 */
                          "   CH#1 FLOW ZERO   ",           /* 1 */
                          "   CH#2 FLOW SPAN   ",           /* 2 */
                          "   CH#2 FLOW ZERO   ",           /* 3 */
                          "   CH#1 K-FACTOR    ",           /* 4 */
                          "   CH#2 K-FACTOR    ",           /* 5 */
                          " CH#1 FLOW CUT-OFF  ",           /* 6 */
                          " CH#2 FLOW CUT-OFF  ",           /* 7 */
                          "  RELAY1 SET-POINT  ",           /* 8 */
                          "  RELAY2 SET-POINT  ",           /* 9 */
                          "   OUTPUT at 4mA    ",           /* 10 */
                          "   OUTPUT at 20mA   ",           /* 11 */
                          "  RELAY DEAD-BAND   " };         /* 12 */

#pragma memory = default

    LCD_pos( 1, 0 );                        LCD_puts( SIGNAL[item] );
    LCD_pos( 2, 0 );                        LCD_puts( BLANK );

    if( item < 4 )
    {
        LCD_pos( 2, 14 );

        LCD_puts( *(Total /*+ Total_unit*/) );
        LCD_puts( *(Rateunit + TimeBase1) );
    }
    else if( item < 6 )
    {
        if( item == 4 )                     item = 0;
        else                                item = 2;

        LCD_pos( 2, 14 );

        LCD_puts( "P/" );
        LCD_puts( *(Total /*+ Total_unit*/) );
    }
    else if( item < 8 )
    {
		LCD_pos( 2, 13 );					
		LCD_putc( '%' );
    }
    else
    {
        LCD_pos( 2, 12 );

        LCD_puts( *(Total /*+ Total_unit*/) );
        LCD_puts( *(Rateunit + TimeBase1) );
    }

    for( i=0 ; i<11 ; i++ )                 dis[i] = ' ';

    switch( item )
    {
        case 0:
        case 1:
        case 2:
        case 3: Option[item+4] %= 1000000000;
                LCD_pos( 2, 3 );            
				digit = 10;
                itoa_converter( Option[item+4], 9, 6 );
                break;




        case 8:
        case 9:
        case 10:
        case 11:Option[item-8] %= 10000000;

                if( Rate_DP1 )
                {
                    LCD_pos( 2, 3 );
                    digit = 8;
                    itoa_converter( Option[item-8], 7, 8-Rate_DP1 );
                }
                else
                {
                    LCD_pos( 2, 4 );
                    digit = 7;
                    itoa_converter( Option[item-8], 7, 0 );
                }
                break;

        case 12:DeadBand %= 100000;

                if( Rate_DP1 )
                {
                    LCD_pos( 2, 5 );
                    digit = 6;
                    itoa_converter( DeadBand, 5, 6-Rate_DP1 );
                }
                else
                {
                    LCD_pos( 2, 6 );
                    digit = 5;
                    itoa_converter( DeadBand, 5, 0 );
                }
                break;
    }

    for( i=0; i<digit ; i++ )
    {
        dis[i] = Dis[i];                    
		LCD_putc( dis[i] );
    }

    LCD_cursorblink( ON );                  
	key_ready( Enter );

    for(;;)
    {
        sync_data( LCDpos );                
		d = dis[x] - '0';

        if( kUp )
        {
            beep();
            LCD_cursorblink( OFF );         
			first = 1;

            d++;                            
			d %= 10;

            dis[x] = d +'0';                LCD_putc( dis[x] );
            key_ready( Up );                LCD_cursorblink( ON );
        }
        else if( kDown )
        {
            beep();
            LCD_cursorblink( OFF );         first = 1;

            if( !d )                        d = 10;
            d--;

            dis[x] = d +'0';                LCD_putc( dis[x] );
            key_ready( Down );              LCD_cursorblink( ON );
        }
        else if( kShift )
        {
            beep();                         first = 1;
            LCDpos++;                       x++;
            if( dis[x] == '.' )
            {
                LCDpos++;                   
				x++;
            }
            if( x == digit )
            {
                x = 0;
                switch( item )
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3: LCD_pos( 2, 3 );break;
                    case 6:
                    case 7: LCD_pos( 2, 7 );break;
                    case 8:
                    case 9:
                    case 10:
                    case 11:if( Rate_DP1 )  LCD_pos( 2, 3 );
                            else            LCD_pos( 2, 4 );
                            break;
                    case 12:if( Rate_DP1 )  LCD_pos( 2, 5 );
                            else            LCD_pos( 2, 6 );
                            break;
                }
            }
            key_ready( Shift );
        }
        else if( kMode )
        {
			beep(); 						
			LCD_cursorblink( OFF );

            if( !first )
            {
				for( i=0; i<digit; i++ )
				{
					if( dis[i] != '.')      dis[i] = '0';

					LCD_putc( dis[i] );
				}
            }
            else
            {
                dis[x] = '0';               
				LCD_putc( dis[x] );
            }
            key_ready( Mode );              
			LCD_cursorblink( ON );
        }
        else if( kEnter )
        {
            beep();
            switch( item )
            {
                case 0:
                case 1:
                case 2:
                case 3: Option[item+4] = 0;
                        for( i=0 ; i<digit ; i++ )
                        {
                            if( dis[i] != '.' )
                            {
                                Option[item+4] += dis[i] - '0';
                                if( i != digit-1 )
                                    Option[item+4] *= 10;
                            }
                        }
                        break;


                case 8:
                case 9:
                case 10:
                case 11:Option[item-8] = 0;
                        for( i=0; i<digit ; i++ )
                        {
                            if( dis[i] != '.' )
                            {
                                Option[item-8] += dis[i] - '0';
                                if( i != digit-1 )
                                    Option[item-8] *= 10;
                            }
                        }
                        if( item == 9 && ( Alarm1 < Alarm2 ) )
                            error = 1;
                        break;

                case 12:DeadBand = 0;
                        for( i=0; i<digit ; i++ )
                        {
                            if( dis[i] != '.' )
                            {
                                DeadBand += dis[i] - '0';
                                if( i != digit-1 )
                                    DeadBand *= 10;
                            }
                        }
                        DB_B = DeadBand;    
						DB_A = ( DeadBand >> 8 );

                        if( ( Alarm1 < DeadBand ) || ( Alarm2 < DeadBand ) )
                            error = 1;
                        break;
                default:
                        break;
            }

            key_ready( Enter );

            if( error )
            {
                LCD_pos( 2, 0 );            
				LCD_puts( "     OVER RANGE!    " );

                Buzz_ON;                    delay( 50000 );
                Buzz_OFF;                   delay( 20000 );
                Buzz_ON;                    delay( 50000 );
                Buzz_OFF;

                error = 0;                  
				x = 0;
                LCD_pos( 2, 0 );            
				LCD_puts( BLANK );

                if( item > 7 )
                {
                    LCD_pos( 2, 12 );

                    LCD_puts( *(Total /*+ Total_unit*/) );
                    LCD_puts( *(Rateunit + TimeBase1) );
                }

                if( item < 12 )
                {
                    if( Rate_DP1 )          LCD_pos( 2, 3 );
                    else                    LCD_pos( 2, 4 );
                }
                else
                {
                    if( Rate_DP1 )          LCD_pos( 2, 5 );
                    else                    LCD_pos( 2, 6 );
                }

                for( i=0; i<digit ; i++ )   LCD_putc( dis[i] );
            }
            else
            {
                LCD_cursorblink( OFF );
                set_ini();
                break;
            }
        }
    }
}

void Set_Flow(void)
{
#pragma memory = xdata

    Char    item=1, view=1, yesno=0;

#pragma memory = code

    Char CAL[][21] =  {     "   OPERATION MODE   ",           /* 0 */
							"   CH#1 FLOW UNIT   ",           /* 1 */
							"   CH#1 TIME BASE   ",           /* 2 */
                            "CH#1 FLOW CORRECTION",           /* 3 */
                            " CH#1 TOTAL DECIMAL ",           /* 4 */
                            " CH#1 RATE DECIMAL  ",           /* 5 */
                            "   CH#2 FLOW UNIT   ",           /* 1 */
                            "   CH#2 TIME BASE   ",           /* 6 */
                            "CH#2 FLOW CORRECTION",           /* 7 */
                            " CH#2 TOTAL DECIMAL ",           /* 8 */
							" CH#2 RATE DECIMAL  ",           /* 9 */
							"  ACC-TOTAL RESET   ",           /* 10 */
							"   DISPLAY ENABLE   ",           /* 11 */
							"  RANGE-OVER CHECK  ",           /* 12 */
							"     BRIGHTNESS     ",           /* 13 */
							"   END OF PROCESS   " };         /* 14 */

    Char CORRCT[][21] = {   "       LINEAR       ",           /* 0 */
                            "     SQUARE ROOT    " };         /* 1 */

    Char DP[][21] =  {      "       00000        ",           /* 0 */
                            "       000.0        ",           /* 1 */
                            "       00.00        ",           /* 2 */
                            "       0.000        " };         /* 3 */



    Char TOT[][21] =  {     "         ltr        ",           /* 0 */
							"         m         ",           /* 1 */
							"         ft        ",           /* 2 */
							"         in        ",           /* 3 */
							"         gal        ",           /* 4 */
							"         kg         ",           /* 5 */
							"         ton        ",           /* 6 */
							"        NONE        " };         /* 7 */
                                                        /* 0x02 */


	Char OPMODE[][21] = {	"  ADDITION(#1+#2)   ",
							" SUBTRACTION(#1-#2) " };

    Char TBASE[][21] = {    "       SECOND       ",           /* 0 */
                            "       MINUTE       ",           /* 1 */
                            "        HOUR        ",           /* 2 */
                            "        DAY         " };         /* 3 */

    Char RESET[][21] = {    "         NO         ",           /* 0 */
                            "         YES        " };         /* 1 */

    Char RETURN[][21] = {   "         NO         ",           /* 0 */
                            "         YES        " };         /* 1 */

#pragma memory = default

    LCD_pos( 2, 0 );                        
	LCD_puts( BLANK );

    key_ready( Enter );

    for(;;)
    {
        if( view )
        {
            view = 0;
            LCD_pos( 1, 0 );                
			LCD_puts( CAL[item] );
            LCD_pos( 2, 0 );

            switch( item )
            {
				/*
                case 0: Op_mode %= 2;
                        LCD_puts( OPMODE[Op_mode] );
                        break;
						*/
				case 1: Total_unit1 %= 8;
                        LCD_puts( TOT[Total_unit1] );
                        break;
				case 2: TimeBase1 %= 4;
						LCD_puts( TBASE[TimeBase1] );
                        break;

                case 4: Total_DP1 %= 4;
						LCD_puts( DP[Total_DP1] );
                        break;
                case 5: Rate_DP1 %= 4;
						LCD_puts( DP[Rate_DP1] );
                        break;

				case 6: Total_unit2 %= 8;
                        LCD_puts( TOT[Total_unit2] );
                        break;

                case 7: TimeBase2 %= 4;
						LCD_puts( TBASE[TimeBase2] );
                        break;

                case 9: Total_DP2 %= 4;
						LCD_puts( DP[Total_DP2] );
                        break;
				case 10: Rate_DP2 %= 4;
						LCD_puts( DP[Rate_DP2] );
                        break;
				case 11:yesno %= 2;
                        LCD_puts( RESET[yesno] );
                        break;
				case 12:AutoReturn %= 2;
                        LCD_puts( RETURN[AutoReturn] );
                        break;
				case 13:Warning %= 2;
                        LCD_puts( RETURN[Warning] );
                        break;

				case 15:LCD_puts( BLANK );
                        break;
            }
        }

        if( kUp )
        {
            beep();                         
			view = 1;

            switch( item )
            {
				/*case 0: Op_mode++;			break; */
				case 1: Total_unit1++;		break;
				case 2: TimeBase1++;		break;

                case 4: Total_DP1++;        break;
                case 5: Rate_DP1++;         break;
				case 6: Total_unit2++;      break;
                case 7: TimeBase2++;        break;

                case 9: Total_DP2++;        break;
				case 10: Rate_DP2++; 		break;
				case 11:yesno++;			break;
				case 12:AutoReturn++;		break;
				case 13:Warning++;			break;

            }
            key_ready( Up );
        }

        else if( kEnter )
        {
            beep();                         
			view = 1;

            switch( item )
            {
                case 2:
#if     Meter1 == PULSE_FLOW
                        Parameter_flow( 4 );    /* K-Factor CH#1 */
                        Parameter_liquid( 8 );  /* Filter Factor CH#1 */
                        item = 3;               /* Skip Correction Setting */
#endif
                        break;

                case 3: Parameter_flow( 6 );    /* Flow Cut-Off CH#1 */
                        Parameter_liquid( 8 );  /* Filter Factor CH#1 */
                        break;

                case 7:
#if     Meter2 == PULSE_FLOW
                        Parameter_flow( 5 );    /* K-Factor CH#2 */
                        Parameter_liquid( 9 );  /* Filter Factor CH#2 */
                        item = 8;               /* Skip Correction Setting */
#endif
                        break;

                case 8: Parameter_flow( 7 );    /* Flow Cut-Off CH#2 */
                        Parameter_liquid( 9 );  /* Filter Factor CH#2 */
                        break;				

				case 11:if( yesno )
                        {
                            ResetValue( 1 );
                            ResetValue( 0 );
                            ResetValue( 3 );
                            ResetValue( 2 );
                        }
						item++;

                        LCD_pos( 2, 0 );    
						LCD_puts( BLANK );
                        break;

                case 15:set_ini();
                        break;
            }

            item++;

#if     Meter1 == PULSE_FLOW && Meter2 == PULSE_FLOW
            if( item == 13 )                item++;
#endif

#if     DisMode == LCD_DISPLAY
			if( item == 14 )				item++;
#endif
			if( item == 16 )				break;
            key_ready( Enter );
        }
    }
}



void Set_Option(void)
{
    Char item=1, view=1;

#pragma memory = code

    Char OPTION[][21] = { "   RELAY SEQUENCE   ",           /* 0 */
                          "     SIGNAL TYPE    ",           /* 1 */
                          "      BAUD RATE     ",           /* 2 */
                          "    DATA LOGGING    ",           /* 3 */
                          "    PRINT METHOD    ",           /* 4 */
                          "   PRINT INTERVAL   ",           /* 5 */
                          "     PRINT UNIT     ",           /* 6 */
                          "    RESET METHOD    ",           /* 7 */
                          "     DATE FORMAT    ",           /* 8 */
                          "      TIME SET      ",           /* 9 */
                          "   END OF PROCESS   " };         /* 10 */

    Char LEVEL[][21] = {  "      HH  /  H      ",
                          "       H  /  L      ",
                          "      L  /  LL      " };

    Char COMM[][21] = {   "       RS-232       ",
                          "       RS-422       ",
                          "       RS-485       " };

    Char BAUD[][21] = {   "        1200        ",
                          "        2400        ",
                          "        4800        ",
                          "        9600        ",
		                  "       19200        " };

    Char METHOD[][21] = { "      COMPUTER      ",
                          "       PRINTER      " };

    Char method[][21] = { "      RESET KEY     ",
                          "    TIME INTERVAL   " };

    Char INTER[][21] = {  "      1 MINUTE      ",           /* 0 */
                          "     10 MINUTES     ",           /* 1 */
                          "     30 MINUTES     ",           /* 2 */
                          "       1 HOUR       ",           /* 3 */
                          "       6 HOURS      ",           /* 4 */
                          "      12 HOURS      ",           /* 5 */
                          "      24 HOURS      " };         /* 6 */

    Char RESET[][21] = {  "      RESET KEY     ",
                          "     PRINT TIME     ",
                          "        24:00       ",
                          "    RESET INHIBIT   " };

    Char DATE[][21] = {   "        KOREA       ",
                          "         USA        ",
                          "       EUROPE       " };

    Char PRINTUNIT[][21]={"        NONE        ",
                          "       DEFALUT      " };

#pragma memory = default

    key_ready( Enter );

    for(;;)
    {
        if( view )
        {
            view = 0;
            LCD_pos( 1, 0 );                
			LCD_puts( OPTION[item] );
            LCD_pos( 2, 0 );

            switch( item )
            {
                case 0: Seq %= 3;
                        LCD_puts( LEVEL[Seq] );
                        break;
                case 1: Signal %= 3;
                        LCD_puts( COMM[Signal] );
                        break;
                case 2: Baud %= 5;
                        LCD_puts( BAUD[Baud] );
                        break;
                case 3: CommType %= 2;
                        LCD_puts( METHOD[CommType] );
                        break;
                case 4: PrintMode %= 2;
                        LCD_puts( method[PrintMode] );
                        break;
                case 5: Interval %= 7;
                        LCD_puts( INTER[Interval] );
                        break;
                case 6: Print_unit %= 2;
                        LCD_puts( PRINTUNIT[Print_unit] );
                        break;
                case 7: ResetMode %= 4;
                        LCD_puts( RESET[ResetMode] );
                        break;
                case 8: DateFormat %= 3;
                        LCD_puts( DATE[DateFormat] );
                        break;
                case 9: Set_time();
                        Parameter_liquid( 7 );
                        item = 10;          view = 1;
                        key_ready( Enter );
                        break;
                case 10:LCD_puts( BLANK );
                        break;
            }
        }

        if( kUp )
        {
            beep();                         
			view = 1;

            switch( item )
            {
                case 0: Seq++;              break;
                case 1: Signal++;           break;
                case 2: Baud++;             break;
                case 3: CommType++;         break;
                case 4: PrintMode++;        break;
                case 5: Interval++;         break;
                case 6: Print_unit++;       break;
                case 7: ResetMode++;
                        if( !CommType || !PrintMode )
                        {
                            if( ResetMode == 1 )
                                ResetMode = 2;
                        }
                        break;
                case 8 :DateFormat++;       
				        break;
            }
            key_ready( Up );
        }
        else if( kEnter )
        {
            beep();                         
			view = 1;

            switch( item )
            {
                case 0: Parameter_flow( 8 );    /* Relay1 Set Point */
                        Parameter_flow( 9 );    /* Relay2 Set Point */
                        Parameter_flow( 12 );   /* Relay Dead-Band  */
                        if( Basic )
                        {
                            item = 9;       
							ResetMode = 0;
                        }
                        else if( Basic_A || Basic_AC )
                        {
                            Parameter_flow( 10 );   /* Output 4mA */
                            Parameter_flow( 11 );   /* Output 20mA */
                            if( Basic_A )
                            {
                                item = 9;   
								ResetMode = 0;
                            }
                        }
                        break;

                case 2: serial_ini( Baud ); 
				        { 
				            item = 7;
							ResetMode = 0;
							CommType = 0;
						}
				        delay( 1000 );
                        break;

                case 3: if( !CommType )
                        {
                            PrintMode = 1;  
							item = 6;
                        }
                        break;

                case 4: if( !PrintMode )    item = 5;
                        break;
            }
            item++;
            if( item == 11 )                break;
            key_ready( Enter );
        }
    }
}

void Set_Test(void)
{
    Char    status=0, item=0, view=1, sign=1,filtering;
    Int     Nowtemp, k;
    int     Offtemp;
    float   value;

#pragma memory = code

    Char TEST[][21] =  {  "CH#1 FREQUENCY INPUT",           /* 0 */
                          " CH#1 ANALOG INPUT  ",           /* 1 */
                          "CH#2 FREQUENCY INPUT",           /* 2 */
                          " CH#2 ANALOG INPUT  ",           /* 3 */
                          " TEMPERATURE INPUT  ",           /* 4 */
                          "   PRESSURE INPUT   ",           /* 5 */
                          "  4mA OUTPUT ADJUST ",           /* 6 */
                          " 20mA OUTPUT ADJUST ",           /* 7 */
                          " RELAY1 ON/OFF TEST ",           /* 8 */
                          " RELAY2 ON/OFF TEST ",           /* 9 */
                          "   END OF PROCESS   " };         /* 10 */

    Char RELAY[][21] = {  "     RELAY  OFF     ",           /* 0 */
                          "     RELAY  ON      " };         /* 1 */

#pragma memory = default

#if     Meter1 == PULSE_FLOW
    filtering = Filter1;                    
    Filter1 = 4;

#endif
    key_ready( Enter );

    for(;;)
    {
        if( view )
        {
            view = 0;
            LCD_pos( 1, 0 );                
			LCD_puts( TEST[item] );

            switch( item )
            {
#if     Meter1 == PULSE_FLOW
                case 0: Nowtemp = PulTotal1;
                        Offtemp = Offset1;  
						EX0 = 1;
                        break;
#endif
#if     Meter2 == PULSE_FLOW
                case 2: Nowtemp = PulTotal2;
                        Offtemp = Offset2;  
						EX1 = 1;
                        break;
#endif
                case 6:
                case 7: DA( Nowtemp );      
                        delay( 5000 );
                        break;
                case 8:
                case 9: LCD_pos( 2, 0 );
                        LCD_puts( RELAY[status] );
                        break;
                default:LCD_pos( 2, 0 );
                        LCD_puts( BLANK );
                        break;
            }
        }

        if( !DisEn )
        {
            LCD_pos( 2, 6 );                
			DisEn = 1;

            switch( item )
            {
#if     Meter1 == PULSE_FLOW
                case 0: value = ( Rate1[0]+Rate1[1]+Rate1[2]+Rate1[3] ) / 4.;
                        LCD_int( 1, PLUS, value*10, 6 );
                        LCD_puts( " Hz" );
                        break;
#endif

#if     Meter2 == PULSE_FLOW
                case 2: value = ( Rate2[0]+Rate2[1]+Rate2[2]+Rate2[3] ) / 4.;
                        LCD_int( 1, PLUS, value*10, 6 );
                        LCD_puts( " Hz" );
                        break;

#endif


                case 6:
                case 7: Nowtemp %= 4096;
                        DA( Nowtemp );              delay( 10000 );
                        break;
            }
        }

        if( kUp )
        {
            beep();
            switch( item )
            {
                case 6:
                case 7: k = 0;
                        LCD_pos( 2, 10 );           LCD_putc( '>' );
                        while( kUp && k<20000 )     k++;

                        if( k>19999 && kUp )
                        {
                            beep();
                            while( kUp )
                            {
                                Nowtemp++;          Nowtemp %= 4096;
                                DA( Nowtemp );      delay( 5000 );
                            }
                        }
                        else                        Nowtemp++;
                        DA( Nowtemp );
                        break;
                case 8:
                case 9: view = 1;
                        status++;                   status %= 2;
                        if( item == 8 )
                        {
                            if( status )            RLY1_ON;
                            else                    RLY1_OFF;
                        }
                        else
                        {
                            if( status )            RLY2_ON;
                            else                    RLY2_OFF;
                        }
                        break;
            }
            key_ready( Up );
        }
        else if( kDown )
        {
            beep();
            if( item == 6 || item == 7 )
            {
                k = 0;
                LCD_pos( 2, 10 );           LCD_putc( '<' );
                while( kDown && k<20000 )   k++;

                if( k > 19999 && kDown )
                {
                    beep();
                    while( kDown )
                    {
                        if( !Nowtemp )      Nowtemp = 4095;
                        else                Nowtemp--;

                        DA( Nowtemp );      delay( 5000 );
                    }
                }
                else
                {
                    if( !Nowtemp )          Nowtemp = 4095;
                    else                    Nowtemp--;
                }
                DA( Nowtemp );
            }
            key_ready( Down );
        }
        else if( kMode )
        {
            beep();
            if( item == 6 )                 Nowtemp = 185;
            else if( item == 7 )            Nowtemp = 4025;
            view = 1;
            key_ready( Mode );
        }
        else if( kEnter )
        {
            beep();                         view = 1;
            LCD_pos( 2, 0 );                LCD_puts( BLANK );

            switch( item )
            {
#if     Meter1 == PULSE_FLOW
                case 0: EX0 = 0;            
                        IE0 = 0;
                        PulTotal1 = Nowtemp;
                        Offset1 = Offtemp;
                        Filter1 = filtering;

#if     Meter2 == PULSE_FLOW
                        filtering = Filter2;
                        Filter2 = 4;        
						item = 1;

#endif
                        break;


#if     Meter2 == PULSE_FLOW
                        filtering = Filter2;
                        Filter2 = 4;        
						item = 1;

#endif
                        break;
#endif
#if     Meter2 == PULSE_FLOW
                case 2: EX1 = 0;            
                        IE1 = 0;
                        PulTotal2 = Nowtemp;
                        Offset2 = Offtemp;
                        Filter2 = filtering;


                        if( Basic_A || Basic_AC )
                            item = 5;
                        else                item = 7;

                        break;

#elif   Meter1 == ANALOG_FLOW
                case 3: ET0 = 0;

#if     Compensation == NONE
                        if( Basic_A || Basic_AC )
                            item = 5;
                        else                item = 7;
#endif
                        break;
#endif
                case 5: if( Basic_A || Basic_AC )
                            item = 5;
                        else                item = 7;
                        break;

                case 6: Zero = Nowtemp;     
				        Nowtemp = Span;
                        break;

                case 7: Span = Nowtemp;     
				        DA( Zero );
                        break;

                case 8:
                case 9:
                        status = 0;
                        RLY1_OFF;           RLY2_OFF;
                        break;
            }
            item++;
            if( item == 6 )                 Nowtemp = Zero;
            else if( item == 11 )
            {

                PreTotal1 = PulTotal1;


                PreTotal2 = PulTotal2;

                break;
            }
            key_ready( Enter );
        }
    }
}

void NVRAM_combined()
{
    ID[0] = ID_A;                           
	ID[1] = ID_B;

/*
    DeadBand = DB_A;
    DeadBand <<= 8;                         
	DeadBand += DB_B;
*/

}

void Set_Menu(void)
{
    Char view=0, item=0;

#pragma memory = code

    Char SetMenu[][21] = { "   FLOW PARAMETER   ",      /* 0 */	                    
                           "       OPTION       ",      /* 1 */
                           "        TEST        ",      /* 2 */
                           "        EXIT        " };    /* 3 */

#pragma memory = default

    for( view=0; view<6 ; view++ )
    {
        LCD_pos( 2, 0 );
        if( view % 2 )                      LCD_puts( VERSION );
        else                                LCD_puts( BLANK );

        delay( 50000 );                     delay( 50000 );
    }

    while( N_kEnter );                      beep();
    key_ready( Enter );
    LCD_clear();

    while( 1 )
    {
        if( view )
        {
            LCD_pos( 1, 0 );                view = 0;
            LCD_puts( SetMenu[item] );
            LCD_pos( 2, 0 );
            if( item == 3 )                 LCD_puts( "    PROGRAM MENU    " );
            else                            LCD_puts( BLANK );
        }

        if( kUp )
        {
            beep();                         view = 1;
            item++;                         item %= 4;
            key_ready( Up );
        }
        else if( kEnter )
        {
            beep();                         view = 1;
            if( item == 3 )
            {
                LCD_clear();
                SSound = 0;                 FSound = 1;
                Pulse = 1;                  DisEn = 0;
                break;
            }
            switch( item )
            {
                case 0: Set_Flow();         break;
                case 1: Set_Option();       break;
                case 2: Set_Test();         break;
            }
            item = 3;
            key_ready( Enter );
        }
    }
}

void sys_ini( void )
{
	/******************************************************
	*													  *
	*		   PORT_A & PORT_B I/O Direction			  *
	*													  *
    *   PA0 : Output( Err/Pul)  PB0 : Output( SEL0 )      *
	*	PA1 : Output( Pulse )	PB1 : Output( SEL1 )	  *
	*	PA2 : Output( Relay1 )	PB2 : Output( SEL2 )	  *
	*	PA3 : Output( Relay2 )	PB3 : Input( Remote SW1 ) *
	*	PA4 : Output( Buzzer )	PB4 : Input( Remote SW2 ) *
	*	PA5 : Input( SW3 )		PB5 : Input( Remote SW3 ) *
	*	PA6 : Input( SW2 )		PB6 : Input( Remote SW4 ) *
	*	PA7 : Input( SW1 )		PB7 : Output( ADC ) 	  *
	*													  *
	******************************************************/


    Buzz_OFF;                               ErrPulse_H;
    RLY1_OFF;                               RLY2_OFF;
    OutPulse_H;                             PulseFlag = 1;
    SWD = 1;                                TXE = 0;
    DIS0 = 0;

	 AUXR |= 0x0c; 

    TXE = 0;         TXE = 0;
    SWD = 1;         SWD = 1;                           
	DIS0 = 0;        DIS0 = 0;

    /******************************************************
	*													  *
    *   T2 State : Timer / 16-bit Auto Reload             *
    *   System Oscillator : 18.432 MHz                    *
    *   Interval : 10ms                                   *
	*													  *
    ******************************************************/
    TH2 = 0xc4;                             TL2 = 0x00;
    RCAP2H = 0xc4;                          RCAP2L = 0x00;
    T2CON = 0x04;                           /* TR2 Set */

    /******************************************************
	*													  *
    *   T0 State : Timer mode#1/gate control disable      *
    *   TCON SET : TR0 & EX0 Set / Edge Triger Mode       *
    *   System Oscillator : 18.432 MHz                    *
    *   Interval : 25ms                                   *
	*													  *
    ******************************************************/
    TH0 = 0x69;                             TL0 = 0xf4;
    TMOD = 0x01;                            TCON = 0x15;

	TR0 = 0;

	EX0 = 0;
	EX1 = 0;

    /******************************************************
	*													  *
    *       Interrupt Enable ( IE : 0xb3 )                *
    *   IE.7(EA)    : Global Interrupt Enable             *
    *   IE.5(ET2)   : Enable Timer 2 Interrupt            *
    *   IE.1(ET0)   : Enable Timer 0 Interrupt            *
    *   IE.0(EX0)   : Enable EXT Timer 0 Interrupt        *
	*													  *
    ******************************************************/
    IE = 0xb7;

	delay(30000);
	delay(30000);
	delay(30000);

    RTC[10] = 0x20;                         /* Register A , BCD format */
    RTC[11] = 0x02;                         /* Register B */

    /******************************************************
    *       Pulse Output & NetTotal Value Initial         *
    ******************************************************/
   
#if     Meter1 == PULSE_FLOW && Meter2 == PULSE_FLOW
    ET0 = 0;

#endif

    /**************************************************
    *        Communication Baud-Rate Setting          *
    **************************************************/
    serial_ini( Baud );                     
	delay( 500 );

    /**************************************************
    *           Communication Buffer Clear            *
    **************************************************/
    memset( &Buf, 0x00, sizeof( ComQueue ));
    Buf.Tail = 0;       
	Buf.Head = 0;       
	Buf.Empty = 0;

    /**************************************************
    *           Detection  System  varibles           *
    **************************************************/
    Total_unit1 %= 8;
	Total_unit2 %= 8;
    Total_DP1 %= 4;                         Total_DP2 %= 4;
    Rate_DP1 %= 4;                          Rate_DP2 %= 4;
    TimeBase1 %= 4;                         TimeBase2 %= 4;
    PrintMode %= 2;                         ResetMode %= 4;
    CommType %= 2;                          Interval %= 7;

}


void main(void)
{
#pragma memory = xdata

    Char    comp1=0, comp2=0, dismode = 0, print, com_enable=1;
    Char    key = 0;
    Char    error = 0;

#pragma memory = data

    Char    i;
    Int     k = 0;
    float   j = 0;

#if     Meter1 == PULSE_FLOW || Meter2 == PULSE_FLOW
    Int     sub = 0;
#endif



#pragma memory = code

    Char    DIS[][21]  = {  "   NET ACC-TOTAL    ",
                            "   CH#1 ACC-TOTAL   ",
                            "   CH#2 ACC-TOTAL   " };

    Char ERR_MSG[][21] = {  " CH#1 SIGNAL ERROR  ",
                            " CH#2 SIGNAL ERROR  " };

#pragma memory = default

    /*******************************************************************
    *                 System & LCD Initialize Function                 *
    *******************************************************************/
    sys_ini();                              NVRAM_combined(); 
    set_ini();                              
	LCD_ini();
	


    AutoReturn = 1;
    /*if (!AutoReturn) 
	{
		LCD_pos( 1, 0 );        
		LCD_puts( "  DISPLAY DISABLED  " );
		LCD_pos( 2, 0 );        
		LCD_puts( BLANK );
	}*/
    
    EX0 = 0;
	EX1 = 0;

    for(;;)
    {
        

        /***************************************************************
        *                CHANNEL #1 TOTAL CALCULATION                  *
        ***************************************************************/


        /*EX0 = 0;                            */
		k = PulTotal1;
        /*EX0 = 1;*/

        if( k > PreTotal1 )
        {
            sub = k - PreTotal1;            
			PreTotal1 += sub;
        }
        else if( PreTotal1 > k )
        {
            sub = 65536-PreTotal1+k;        
			PreTotal1 = k;
        }
        else                                sub = 0;

        if( sub )
        {
            j = sub * Total_Fac1;           GrossRemain1 += j;
            Total = GrossRemain1;           GrossRemain1 -= Total;
            GrossTotal1 += Total;           GrossTotal1 %= 100000000;
        }



        /***************************************************************
        *                CHANNEL #2 TOTAL CALCULATION                  *
        ***************************************************************/


        /*EX1 = 0;                            */
		k = PulTotal2;
        /*EX1 = 1;*/

        if( k > PreTotal2 )
        {
            sub = k - PreTotal2;            
			PreTotal2 += sub;
        }
        else if( PreTotal2 > k )
        {
            sub = 65536-PreTotal2+k;        
			PreTotal2 = k;
        }
        else                     sub = 0;

        if( sub )
        {
            j = sub * Total_Fac2;           GrossRemain2 += j;
            Total = GrossRemain2;           GrossRemain2 -= Total;
            GrossTotal2 += Total;           GrossTotal2 %= 100000000;
        }

       /* EX0 = 0;
		EX1 = 0;
*/
	  /* pulse output not use 
        if( OldTotal >= 10000000 )
        {
            Pulse = 0;                      
			OldTotal %= 10000000;
            Pulse = 1;

        }
         */


        /***************************************************************
        *                 CHANNEL #1 Rate Caculation                   *
        ***************************************************************/
        RATE = 0;
        for( i=0 ; i<Filter1 ; i++ )        RATE += Rate1[i];
      

        DisRate1 = ( RATE * Rate_Fac1 / Filter1 ) + RateHalf1;
      
            

        /***************************************************************
        *                 CHANNEL #2 Rate Caculation                   *
        ***************************************************************/
        RATE = 0;
        for( i=0 ; i<Filter2 ; i++ )        RATE += Rate2[i];
      

        DisRate2 = ( RATE * Rate_Fac2 / Filter2 ) + RateHalf2;
       
            

     
        /***************************************************************
        *                       Program Menu                           *
        ***************************************************************/
        if( kUp )                      /* Shift & Down */
        {
            beep();                         
			k = 0;
			EX0 = 1;
			EX1 = 1;

            while( kUp && k < 60000 )  k++;

            if( kUp && k > 59999 )
            {
                EX0 = 0;                    
				EX1 = 0;
                ET0 = 0;
			

                beep();

                LCD_pos( 1, 0 );            LCD_puts( MODEL );
                LCD_pos( 2, 0 );            Option_check();
                key_ready( Up );       
				

                RLY1_OFF;                   RLY2_OFF;
                Rly1_flag = 0;              Rly2_flag = 0;

                delay( 1000 );              
				Pulse = 0;

                Set_Menu();       

			
				com_enable = 1;

                for(i=0 ; i<Filter1 ; i++)  Rate1[i] = 0;
                for(i=0 ; i<Filter2 ; i++)  Rate2[i] = 0;

                dismode = 0;
                DisRate1 = 0;               
				DisRate2 = 0;
                


                key_ready( Enter );

#if     Meter1 == PULSE_FLOW
                IE0 = 0;                    
/*                EX0 = 1;*/
#endif

#if     Meter2 == PULSE_FLOW
                IE1 = 0;                    
/*                EX1 = 1;*/
#endif
            }
            /*if (!AutoReturn) 
	        {
		        LCD_pos( 1, 0 );        
		        LCD_puts( "  DISPLAY DISABLED  " );
		        LCD_pos( 2, 0 );        
		        LCD_puts( BLANK );
	        }*/


        }
		else if( kDown )
		{
			beep();
			EX0 = 0;
            EX1 = 0;
			ET0 = 0;
			/*ResetValue(1);
			dismode = 0;                    
			DisEn = 0;
            LCD_clear();
*/
           
			key_ready( Down );

#if     Meter1 == PULSE_FLOW
            IE0 = 0;                        
/*            EX0 = 1;*/
#endif
#if     Meter2 == PULSE_FLOW
            IE1 = 0;                        
/*            EX1 = 1;*/

#endif
     	    EX0 = 0;
            EX1 = 0;
	    }
        /***************************************************************
        *              GrossTotal1 & GrossTotal2 Reset                 *
        ***************************************************************/
        else if( kEnter/* || !( R_Reset ) */)
        {

			/*
            if( !( R_Reset ) )              key = 1;
            else                            key = 0;
            */

            beep();

            EX0 = 0;                        
			EX1 = 0;
            ET0 = 0;

			/*while( kSetting && k < 30000 )  k++;

            if( kSetting && k > 29999 )
            {*/
				ResetValue( 3 );
				ResetValue( 1 );
			/*}*/

            /*
            if( Basic_C || Basic_AC )
            {
                if( !ResetMode || !PrintMode )
                {
                    if( CommType && !PrintMode )       Printing();

                    if( !ResetMode )
                    {
                        ResetValue( 1 );    
						ResetValue( 3 );
                    }
                }
            }
            else*/
            /*{
                ResetValue( 1 );            
				ResetValue( 3 );
            }*/

            dismode = 0;                    
			DisEn = 0;
            LCD_clear();

           /* if( key )                       remote_ready( Reset );
            else                           */ 
			key_ready( Enter );

#if     Meter1 == PULSE_FLOW
            IE0 = 0;                        
/*            EX0 = 1;*/
#endif
#if     Meter2 == PULSE_FLOW
            IE1 = 0;                        
/*            EX1 = 1;*/

#endif

            /*if (!AutoReturn) 
	        {
		        LCD_pos( 1, 0 );        
		        LCD_puts( "  DISPLAY DISABLED  " );
		        LCD_pos( 2, 0 );        
		        LCD_puts( BLANK );
	        }*/


        }

 
        /***************************************************************
        *                    Display Mode Select                       *
        ***************************************************************/
        else if( kMode )
        {
            beep();
            if( SSound )
            {
                FSound = 0;                 
				SSound = 0;
            }

            dismode++;                      dismode %= 2;
            DisEn = 0;                      DisTimer = 0;
            key_ready( Mode );              
			LCD_clear();

            /*if (!AutoReturn) 
	        {
		        LCD_pos( 1, 0 );        
		        LCD_puts( "  DISPLAY DISABLED  " );
		        LCD_pos( 2, 0 );        
		        LCD_puts( BLANK );
	        }*/

        }


        /*if (!AutoReturn)		DisEn = 1;*/
		
        /***************************************************************
        *              GrossTotal/Rate/Acc-Total Display               *
        ***************************************************************/
        if( !DisEn )
        {
            switch( dismode )
            {
                
                case 0:
                    LCD_pos( 1, 0 );        
				    LCD_puts( "CH#1" );

                    /*LCD_pos( 1, 6 );
                    LCD_int( Rate_DP1, PLUS, DisRate1, 7 );
                    if( Total_unit1 != 7 )
                    {
                        LCD_puts( *(Totaldis + Total_unit1) );
                        LCD_puts( *(Rateunit + TimeBase1) );
                    }*/

					LCD_pos( 2, 0 );        
				    LCD_puts( "CH#2" );

                    if( !error )
                    {
                        LCD_pos( 1, 6 );
                        LCD_int( Total_DP1, PLUS, GrossTotal1, 8 );
                        if( Total_unit1 != 7 )
                            LCD_puts( *(Totaldis + Total_unit1) );

						LCD_pos( 2, 6 );
                        LCD_int( Total_DP2, PLUS, GrossTotal2, 8 );
                        if( Total_unit2 != 7 )
                            LCD_puts( *(Totaldis + Total_unit2) );
                    }
                    break;

/* You know what i'm saying 
                case 2:
					LCD_pos (2, 0);
				    LCD_puts ("CH#1" );

					LCD_pos (1, 6);
					LCD_int (Rate_DP1, PLUS, DisRate1, 7 );
					if (Total_unit != 7)
					{
						LCD_puts ( *(Totaldis + Total_unit) );
						LCD_puts ( *(Rateunit + TimeBase1) );

					}
					break;
*/
                case 1:
					LCD_pos( 1, 0 );        
				    LCD_puts( "CH#1" );

                    LCD_pos( 1, 6 );
                    LCD_int( Rate_DP1, PLUS, DisRate1, 7 );
                    if( Total_unit1 != 7 )
                    {
                        LCD_puts( *(Totaldis + Total_unit1) );
                        LCD_puts( *(Rateunit + TimeBase1) );
                    }

                    LCD_pos( 2, 0 );        
				    LCD_puts( "CH#2" );

                    LCD_pos( 2, 6 );
                    LCD_int( Rate_DP2, PLUS, DisRate2, 7 );
                    if( Total_unit2 != 7 )
                    {
                        LCD_puts( *(Totaldis + Total_unit2) );
                        LCD_puts( *(Rateunit + TimeBase2) );
                    }

                    /*if( !error )
                    {
                        LCD_pos( 2, 5 );
                        LCD_int( Total_DP2, PLUS, GrossTotal2, 8 );
                        if( Total_unit2 != 7 )
                            LCD_puts( *(Totaldis + Total_unit2) );
                    }*/
                    break;

            
                case 2:
                    LCD_pos( 1, 0 );        
				    LCD_puts( DIS[1] );

                    if( !error )
                    {
                        LCD_pos( 2, 4 );
                        LCD_int( Total_DP1, PLUS, GrossTotal1+GrossAccTotal1, 8 );
                        if( Total_unit1 != 7 )
                            LCD_puts( *(Totaldis + Total_unit1) );
                    }
                    break;

				case 3:
                    LCD_pos( 1, 0 );        
				    LCD_puts( DIS[2] );

                    if( !error )
                    {
                        LCD_pos( 2, 4 );
                        LCD_int( Total_DP2, PLUS, GrossTotal2+GrossAccTotal2, 8 );
                        if( Total_unit2 != 7 )
                            LCD_puts( *(Totaldis + Total_unit2) );
                    }
                    break;
            }
            DisEn = 1;
        }


        /***************************************************************
        *                    InterLock status flag                     *
        ***************************************************************/
         
        if(NR_Interlock1) interlock1_flag = 1;  /* 접지 단선이면 1 */
		else              interlock1_flag = 0;  /* 접지 정상연결 0 */

		if(NR_Interlock2) interlock2_flag = 1;
		else              interlock2_flag = 0;
		

        /***************************************************************
        *           Interval Printting & Serial Communication          *
        ***************************************************************/
      /*  if( Basic_C || Basic_AC )
        {   
                
            if( !CommType ) */                /* LOGGING BY COMPUTER */
                communication();
     /*   }                                                                    */

     
        /***************************************************************
        *                        Analog Output                         *
        ***************************************************************/
		/*
        if( Basic_A || Basic_AC )
        {
            if( DisNetRate < ZeroAlarm )        DA( Zero );
            else if( DisNetRate > SpanAlarm )   DA( Span );
            else
                DA( ( DisNetRate - ZeroAlarm ) * Analog + Zero );
        }
		*/
    }
}

        /***************************************************************
        *                      Interrupt Routine                       *
        ***************************************************************/



interrupt [0x03] void EX0_int ( void )
{
    PulTotal1++;
}




interrupt [0x13] void EX1_int ( void )
{
    PulTotal2++;
}

/*
interrupt [0x0B] void T0_int ( void )       * 25msec interval *
{
    TH0 = 0x6a;                             
	TL0 = 0x00;

    DisCount++;                             
	DisCount %= 40;

    if( !DisCount )
    {

    }
}
*/


interrupt [0x2B] void T2_int ( void )       /* 10msec interval */
{
    TF2 = 0;

    if( DisEn )
    {
        DisEn++;                            
		DisEn %= 26;
    }

    
    DisCount++;                             
	DisCount %= 100;

    if( !DisCount )
    {
        DisTimer++;


        /* Pulse Channel #1  Caculate */
        TempTotal = PulTotal1;

        if( TempTotal > Offset1 )
            Rate1[FilterCounter1] = TempTotal - Offset1;
		else if( Offset1 > TempTotal )
            Rate1[FilterCounter1] = 65536 - Offset1 + TempTotal;
		else								Rate1[FilterCounter1] = 0;

        Offset1 = TempTotal;                
		FilterCounter1++;
		FilterCounter1 %= Filter1;
        /*******************************/

 
        /* Pulse Channel #2 Caclulate */
		TempTotal = PulTotal2;

		if( TempTotal > Offset2 )
			Rate2[FilterCounter2] = TempTotal - Offset2;
		else if( Offset2 > TempTotal )
			Rate2[FilterCounter2] = 65536 - Offset2 + TempTotal;
		else								Rate2[FilterCounter2] = 0;

		Offset2 = TempTotal;				
		FilterCounter2++;
        FilterCounter2 %= Filter2;
        /*******************************/
    }

    /* Pulse OUtput not use
    if( Pulse )
    {
        if( OldTotal == NetTotal )          PulseFlag = 1;
        else
        {
            if( !PulseFlag )
            {
                OldTotal++;
                OutPulse_H;                 
				PulseFlag = 1;
            }
            else
            {
                OutPulse_L;                 
				PulseFlag = 0;
            }
        }
    }
	*/
}


interrupt [0x23] void SCON0_int (void)      /* Serial Port */
{
    Buf.Buff[Buf.Head++] = serial_getc();   
	Buf.Empty = 1;
    Buf.Head %= BUFSIZE;
}



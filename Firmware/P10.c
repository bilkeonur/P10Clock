#include <16F648A.h>
#fuses HS,NOWDT,NOBROWNOUT,NOLVP,NOPUT,PROTECT,CPD
#use delay(clock=20000000)

#rom 0x2100={0,0,60,126,102,102,102,102,102,102,102,102,126,60,0,0} //0
#rom 0x2110={0,0,126,126,24,24,24,24,24,24,24,120,56,24,0,0} //1
#rom 0x2120={0,0,60,126,96,96,96,126,126,6,6,6,126,60,0,0} //2
#rom 0x2130={0,0,60,126,70,6,6,62,62,6,6,70,126,60,0,0} //3
#rom 0x2140={0,0,6,6,6,6,6,126,126,102,102,102,102,102,0,0} //4
#rom 0x2150={0,0,60,126,102,6,6,126,124,96,96,96,126,126,0,0} //5
#rom 0x2160={0,0,60,126,102,102,102,126,124,96,96,102,126,60,0,0} //6
#rom 0x2170={0,0,48,48,48,48,48,48,24,12,6,6,126,126,0,0} //7
#rom 0x2180={0,0,60,126,102,102,102,60,60,102,102,102,126,60,0,0} //8
#rom 0x2190={0,0,60,126,102,6,6,62,126,102,102,102,126,60,0,0} //9
#rom 0x21A0={0,0,0,0,0,0,0,0,0,60,102,102,102,60,0,0} //(°)
#rom 0x21B0={0,0,28,62,114,96,96,96,96,96,96,114,62,28,0,0} //(C)
#rom 0x21C0={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} //(Space)
#rom 0x21D0={0,0,0,0,0,0,0,126,126,0,0,0,0,0,0,0} //(-)
#rom 0x21E0={0} //Size

#include <DS1307.c>
#include <1wire.c>

#define OE pin_b6
#define A pin_b5
#define B pin_b4
#define CLK pin_a2
#define LE pin_a3
#define RI1 pin_b7
#define RI2 pin_a1
#define RI3 pin_a0
#define SET pin_b0
#define UP pin_b1

BYTE sec; BYTE min; BYTE hrs; BYTE day; BYTE month; BYTE yr; BYTE dow;
signed int8 temperature=0;

int1 clkDots=1; //clock Dots
int1 dateDots=0; //Date Dots

int8 clkMode=0;

int dispCnt=0;

int8 numData[16];

int8 numbers[4]={12,12,12,12};
int8 nm;

int16 mode=0;
int1 clkSep=0;
int1 tmpRead=0;

int8 editMode=0;
int8 editCnt=0;
int8 scan=0;

int8 setNum[12];
int8 setCnt=0;
int8 scrCnt=0;

int1 shift=0;
int8 setMax=0;

void clock()
{
   output_high(CLK);
   output_low(CLK);
}

void latchEnable()
{
   output_high(LE);
   output_low(LE);
}

void write274HC595(int8 outputData1)
{         
   for(int8 i=0;i<=7;i++)
   {
      if (outputData1 & 0x80)
      {
         output_low(RI1);        
      }
      else
      {
         output_high(RI1);
      }
      
      clock();

      outputData1 <<= 1;
   }
   
   latchEnable();
}

void write274HC595(int8 outputData1 , int8 outputData2 , int8 mode)
{         
   if(mode==1)
   {
      for(int8 l=0;l<=3;l++)
      {
         outputData1 <<= 1;
         outputData2 <<= 1;
      }
   }
   
   for(int8 i=0;i<=3;i++)
   {
      if (outputData1 & 0x80)
      {
         output_low(RI1);        
      }
      else
      {
         output_high(RI1);
      }
      
      if (outputData2 & 0x80)
      {
         output_low(RI2);        
      }
      else
      {
         output_high(RI2);
      }
      
      clock();
      clock();

      outputData1 <<= 1;
      outputData2 <<= 1;
   }
   
   latchEnable();
}

void write274HC595(int8 outputData1 , int8 outputData2 , int8 outputData3 , int8 mode)
{         
   if(mode==1)
   {
      for(int8 l=0;l<=1;l++)
      {
         outputData1 <<= 1;
         outputData2 <<= 1;
         outputData3 <<= 1;
      }
   }
   
   if(mode==2)
   {
      for(int8 l=0;l<=3;l++)
      {
         outputData1 <<= 1;
         outputData2 <<= 1;
         outputData3 <<= 1;
      }
   }
   
   if(mode==3)
   {
      for(int8 l=0;l<=5;l++)
      {
         outputData1 <<= 1;
         outputData2 <<= 1;
         outputData3 <<= 1;
      }
   }
   
   for(int8 i=0;i<=1;i++)
   {
      if (outputData1 & 0x80)
      {
         output_low(RI1);        
      }
      else
      {
         output_high(RI1);
      }
      
      if (outputData2 & 0x80)
      {
         output_low(RI2);        
      }
      else
      {
         output_high(RI2);
      }
      
      if (outputData3 & 0x80)
      {
         output_low(RI3);        
      }
      else
      {
         output_high(RI3);
      }
      
      clock();
      clock();
      clock();
      clock();
      
      outputData1 <<= 1;
      outputData2 <<= 1;
      outputData3 <<= 1;
   }
   
   latchEnable();
}

void createNumber(int line)
{   
   for (int8 k=0;k<=3;k++)
   {
      nm=numbers[k];
      
      for(int8 o=0;o<=15;o++)
      {
         numData[o]=read_eeprom((nm*16)+o);
      }
      
      if(shift==1)
      {
         for(int8 m=0;m<=15;m++)
         {
            if(k<=1)
            {
               numData[m]<<= 1;
            }
            else
            {
               numData[m]>>= 1;
            }
         }
      }
      
      if (clkDots==1)
      {
         if(k==1)
         {
            numData[10]=numData[10]+1;numData[9]=numData[9]+1;
            numData[6]=numData[6]+1;numData[5]=numData[5]+1;
         }
               
         if (k==2)
         {
            numData[10]=numData[10]+128;numData[9]=numData[9]+128;
            numData[6]=numData[6]+128;numData[5]=numData[5]+128;
         }
      }
      
      if(dateDots==1)
      {
         if(k==1)
         {
            numData[2]=numData[2]+1;numData[3]=numData[3]+1;
         }
         
         if (k==2)
         {
            numData[2]=numData[2]+128;numData[3]=numData[3]+128;
         }  
      }
      
      if(scan==0)//16x32
      {
         for(int8 j=3;j<=15;j=j+4)
         {
            write274HC595(numData[j-line]);
         }
      }
      
      if(scan==1) //32x64
      {     
         if(line%2==0)
         {
            for(int8 j=1;j<=7;j=j+2)
            {
               write274HC595(numData[(j+8)-(line/2)],numData[j-(line/2)],0);
            }
                  
            for(j=1;j<=7;j=j+2)
            {
               write274HC595(numData[(j+8)-(line/2)],numData[j-(line/2)],1);
            }
         }
      }        
      
      if(scan==2) //48x128
      {      
         if(line%4==0)
         {
            for(int8 j=2;j<=5;j++)
            {
               write274HC595(numData[j+8],numData[j+4],numData[j],0);
            }
                  
            for(j=2;j<=5;j++)
            {
               write274HC595(numData[j+8],numData[j+4],numData[j],1);
            }
                  
            for(j=2;j<=5;j++)
            {
               write274HC595(numData[j+8],numData[j+4],numData[j],2);
            }
                  
            for(j=2;j<=5;j++)
            {
               write274HC595(numData[j+8],numData[j+4],numData[j],3);
            }
         }
      }
   }
}

void getTime()
{
   ds1307_get_time(hrs,min,sec);
   ds1307_get_date(day,month,yr,dow);
}

void show(int8 md)
{
   getTime();
   
   switch (md)
   {
      case 0:
         
         clkSep=1;
         dateDots=0;
         shift=1;
         
         numbers[0]=hrs/10;
         numbers[1]=hrs%10;
         numbers[2]=min/10;
         numbers[3]=min%10;
         
         break;
         
      case 1:
                  
         clkSep=0;
         clkDots=0;
         dateDots=1;
         shift=1;
         
         numbers[0]=day/10;
         numbers[1]=day%10;
         numbers[2]=month/10;
         numbers[3]=month%10;
            
         break;
         
      case 2:
                 
         clkSep=0;
         clkDots=0;
         dateDots=0;
         shift=0; 
            
         numbers[0]=2;
         numbers[1]=0;
         numbers[2]=yr/10;
         numbers[3]=yr%10;
           
         break;
         
      case 3:
         
         if(tmpRead==0)
         {
            output_low(OE);
            delay_ms(5);
            temperature = ds1820_read()/10;
            tmpRead=1;
         }
         
         clkSep=0;
         clkDots=0;
         dateDots=0;
         shift=0; 
          
         if(temperature>=0 && temperature<100)
         {
            numbers[0]=temperature/10;
            numbers[1]=temperature%10;
            numbers[2]=10;
            numbers[3]=11;
         }
         else if(temperature<0 && temperature>-100)
         {
            numbers[0]=13;
            numbers[1]=temperature/10;
            numbers[2]=temperature%10;
            numbers[3]=10;
         }
         else
         {
            numbers[0]=13;
            numbers[1]=13;
            numbers[2]=13;
            numbers[3]=13;
         }
         
         break;
   }
}

#int_timer0
void timer0_kesme()
{
   set_timer0(255);
   
   switch (dispCnt)
   {
      case 0:
         output_low(OE);createNumber(dispCnt);
         output_low(A);output_low(B);output_high(OE);
         break;
   
      case 1:
         output_low(OE);createNumber(dispCnt);
         output_high(A);output_low(B);output_high(OE);
         break;
   
      case 2:
         output_low(OE);createNumber(dispCnt);
         output_low(A);output_high(B);output_high(OE);
         break;
   
      case 3:
         output_low(OE);createNumber(dispCnt);
         output_high(A);output_high(B);output_high(OE);
         break;
   }
   
   dispCnt++;if(dispCnt>3)dispCnt=0;
   
   if(editMode==0)
   {
      if(clkSep==1) //Clock Dots
      {
         if(clkMode>=0 && clkMode<150)
         {
            clkDots=1;
         }
         else if(clkMode>=150 && clkMode<300)
         {
            clkDots=0;
         }
         else if(clkMode>=300)
         {
            clkMode=0;
         }
      
         clkMode++;
      }
   }
   
   if(editMode==0)
   {        
      if(mode<=3000)
      {
         tmpRead=0;
         show(0);
      }
      else if(mode>3000 && mode<=6000)
      {
         show(1);
      }
      else if(mode>6000 && mode<=9000)
      {
         show(2);
      }
      else if(mode>9000 && mode<=12000)
      {
         show(3);
      }
      else if(mode>12000)
      {
         mode=0;
      }
            
      mode++;
   }
   else
   {
      if(editCnt>=0 && editCnt<100)
      {
         numbers[scrCnt]=12;
      }
      else if(editCnt>=100 && editCnt<200)
      {
         numbers[scrCnt]=setNum[setCnt];
      }
      else if(editCnt>=200)
      {
         editCnt=0;
      }
      
      editCnt++;
   }
}

void setTime(int8 hour , int8 minute , int8 day , int8 month , int8 year)
{
   ds1307_set_date_time(day,month,year,1,hour,minute,00);
}

void setNumbers()
{
   getTime();

   setNum[0]=hrs/10;
   setNum[1]=hrs%10;
   setNum[2]=min/10;
   setNum[3]=min%10;
   setNum[4]=day/10;
   setNum[5]=day%10;
   setNum[6]=month/10;
   setNum[7]=month%10;
   setNum[8]=yr/10;
   setNum[9]=yr%10;
}

void btnDelay()
{
   for(int16 n=1;n<=1000;n++)
   {
      delay_us(1);
   }
}

void readSizeSettings()
{
   scan=read_eeprom(224);   
}

void writeSizeSettings()
{
   write_eeprom(224,scan);
}

void readSetButton()
{
   if(input(Set))
   {
      mode=0;
      clkSep=0;
      clkDots=0;
      dateDots=0;
      clkMode=0;
      
      editCnt=0;
      setCnt=editMode;
      scrCnt=editMode%4;
      
      switch (editMode)
      {
         case 0:
            clkSep=0;
            clkDots=1;
            dateDots=0;
            setNumbers();
            numbers[0]=setNum[0];numbers[1]=setNum[1];numbers[2]=setNum[2];numbers[3]=setNum[3];
            setMax=2;
            editMode=1;
            break;
               
        case 1:
            clkSep=0;
            clkDots=1;
            dateDots=0;
            numbers[0]=setNum[0];numbers[1]=setNum[1];numbers[2]=setNum[2];numbers[3]=setNum[3];
            setMax=9;
            editMode++;
            break;
               
         case 2:
            clkSep=0;
            clkDots=1;
            dateDots=0;
            numbers[0]=setNum[0];numbers[1]=setNum[1];numbers[2]=setNum[2];numbers[3]=setNum[3];
            setMax=5;
            editMode++;
            break;
               
         case 3:
            clkSep=0;
            clkDots=1;
            dateDots=0;
            numbers[0]=setNum[0];numbers[1]=setNum[1];numbers[2]=setNum[2];numbers[3]=setNum[3];
            setMax=9;
            editMode++;
            break;
            
         case 4:
            clkSep=0;
            clkDots=0;
            dateDots=1;
            numbers[0]=setNum[4];numbers[1]=setNum[5];numbers[2]=setNum[6];numbers[3]=setNum[7];
            setMax=3;
            editMode=5;
            break;
               
         case 5:
            clkSep=0;
            clkDots=0;
            dateDots=1;
            numbers[0]=setNum[4];numbers[1]=setNum[5];numbers[2]=setNum[6];numbers[3]=setNum[7];
            setMax=9;
            editMode++;
            break;
               
         case 6:
            clkSep=0;
            clkDots=0;
            dateDots=1;
            numbers[0]=setNum[4];numbers[1]=setNum[5];numbers[2]=setNum[6];numbers[3]=setNum[7];
            setMax=1;
            editMode++;
            break;
               
         case 7:
            clkSep=0;
            clkDots=0;
            dateDots=1;
            numbers[0]=setNum[4];numbers[1]=setNum[5];numbers[2]=setNum[6];numbers[3]=setNum[7];
            setMax=9;
            editMode++;
            break;
               
         case 8:
            clkSep=0;
            clkDots=0;
            dateDots=0;
            numbers[0]=2;numbers[1]=0;numbers[2]=setNum[8];numbers[3]=setNum[9];
            scrCnt=2;
            setMax=9;
            editMode=9;
            break;
             
         case 9:
            clkSep=0;
            clkDots=0;
            dateDots=0;
            numbers[0]=2;numbers[1]=0;numbers[2]=setNum[8];numbers[3]=setNum[9];
            scrCnt=3;
            setMax=9;
            editMode++;
            break;
               
         case 10:
            setTime((setNum[0]*10)+setNum[1],(setNum[2]*10)+setNum[3],(setNum[4]*10)+setNum[5],(setNum[6]*10)+setNum[7],(setNum[8]*10)+setNum[9]);
            mode=0;
            editMode=0;
            break;
      }
      
      while(input(SET))
      {  
         btnDelay();
      } 
   }
}

void readUpButton()
{
   if(input(UP))
   {
      mode=0;
      
      if(editMode!=0)
      {
         if(setNum[setCnt]==setMax)
         {
            setNum[setCnt]=0;
         }
         else
         {
            setNum[setCnt]++;
         }
      }
      else
      {
         scan++;
      
         if(scan==3)
         {
            scan=0;
         }
      
         writeSizeSettings();
         readSizeSettings();
      }
      
      while(input(UP))
      {
         btnDelay();
      }
   }
}

void main()
{
   setup_timer_1(T1_DISABLED);     
   setup_timer_2(T2_DISABLED,0,1);             
   setup_CCP1(CCP_OFF);
   
   setup_timer_0(RTCC_INTERNAL | RTCC_8_BIT | RTCC_DIV_32);
   set_timer0(255);
   
   set_tris_b(0b00000111);
   output_b(0);
   
   enable_interrupts(INT_TIMER0);
   enable_interrupts(GLOBAL);
     
   ds1307_init();
   
   readSizeSettings();
         
   while(1)
   {      
      readSetButton();
      readUpButton();
   } 
}

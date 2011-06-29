// set up pins connected to 74HC595 chain
int clockPin =  8;
int dataPin  = 11;
int latchPin = 12;

void setup()
{
  // Serial.begin(9600);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin,  OUTPUT);
  pinMode(latchPin, OUTPUT);
  digitalWrite(clockPin, 0);
  digitalWrite(dataPin,  0);
  digitalWrite(latchPin, 0);
}

 
// shift 32 bits into74HC595 chain, MSB first
void shiftOut32(unsigned long data)
{
  int i=0;
  unsigned long one = 1;
  int pinState;
    
  for (i=31; i>=0; i--)  
  {
    if ( data & (one<<i) ) {
      pinState= 0;
    }
    else {	
      pinState= 1;
    }
    digitalWrite(dataPin,  pinState);
    digitalWrite(clockPin, 1);
    digitalWrite(clockPin, 0);
  }
  digitalWrite(latchPin, 1);
  digitalWrite(latchPin, 0);
}

void cylon(int length, int dly)
{
  unsigned long i = 1;
  int j;
  
  for (j = 0; j < length; j++) 
  {
    // Serial.println(i);
    shiftOut32(i); 
    i = i<<1;
    delay(dly);
  }
  for (j = 0; j < length; j++)
  {
    // Serial.println(i);
    shiftOut32(i); 
    i = i>>1;
    delay(dly);
  }
  return;
}

void blinkem(int dly)
{
  shiftOut32(1<<32); 
  delay(dly);
  shiftOut32(0); 
  delay(dly);
}

void blink1(unsigned long which, int dly)
{
  shiftOut32(which); 
  delay(dly);
  shiftOut32(0); 
  delay(dly);
}

void pat1(int dly)
{
  shiftOut32(0x8888); 
  delay(dly);
  shiftOut32(0x4444);
  delay(dly);
  shiftOut32(0x2222); 
  delay(dly);
  shiftOut32(0x1111);
  delay(dly);
  shiftOut32(0x2222); 
  delay(dly);
  shiftOut32(0x4444);
  delay(dly);
}
 
void pat2(int dly)
{
  shiftOut32(0x00008421); 
  delay(dly);
  shiftOut32(0x00004212);
  delay(dly);
  shiftOut32(0x00002124); 
  delay(dly);
  shiftOut32(0x00001248);
  delay(dly);
  shiftOut32(0x00002484); 
  delay(dly);
  shiftOut32(0x00004842);
  delay(dly);
} 

void rnd(int dly)
{
  long i;
  i=random(0xffff);
  shiftOut32(i);
  delay(dly);
}

unsigned long crtl(int col, int row)
{
   return (1 << (((unsigned long)col * 4) + (unsigned long)row));
}    

void test_pat(int mx, int dly)
{
  long row, col;
  int  count;
  
  for(count = 0; count < mx; count++)
  {
    for(col = 0; col < 4; col++)
    {
      for(row = 0; row < 4; row++)
      {
         shiftOut32(crtl(col, row));
         delay(dly);
      }
      for(row = 3; row >= 0; row--)
      {
         shiftOut32(crtl(col, row));
         delay(dly);
      }
    }
  }
}    

void binary(int dly)
{
  long col;
 
  for(col = 0; col < 65536; col+=4)
  {
     shiftOut32(col);
     delay(dly);
  }
} 

void line_up_down(void)
{
  long row, col;
  int count;
  
  for(count = 0; count < 16; count++)
  {
    for(row = 0; row < 4; row++)
    {
       shiftOut32(crtl(0, row) | crtl(1, row) | crtl(2, row) | crtl(3, row));
       delay(125);
    }
    for(row = 3; row >= 0; row--)
    {
       shiftOut32(crtl(0, row) | crtl(1, row) | crtl(2, row) | crtl(3, row));
       delay(125);
    }
  }
} 

void line_round(void)
{
  long row, col;
  int count;
  
  for(count = 0; count < 16; count++)
  {
    for(col = 0; col < 4; col++)
    {
       shiftOut32(crtl(col, 0) | crtl(col, 1) | crtl(col, 2) | crtl(col, 3));
       delay(125);
    }
  }
} 

void seizure(void)
{
  int count, m;
  
    for(m = 0; m< 100; m++)
    {
       shiftOut32((m % 2) == 1 ? 65535 : 0);
       delay(75);
    }
}

void spiral(void)
{
  int count = 0, i;
  
    for(i = 0; count < 20; i+=5)
    {
       if (i > 15)
       {
          i = i % 16;
          count++;
       }
       shiftOut32(1 << i);
       delay(150);
    }
}

void random_fill(void)
{
  int count, i, out = 0, old = 0;
  
  for(count = 0; count < 10; count++)
  {
    out = 0;
    for(i = 0; i < 16; i++)
    {
       while(1)
       {
          old = out;
          //if (count % 2 == 0)
              out |= (1 << random(16));
          //else
          //    out &= ~(1 << random(16));
          if (old != out) break; 
       }   
       shiftOut32(out);
       delay(150);
    }
  }
}

void poutput(unsigned long p, int dly)
{
  unsigned long o = random(0xFFFF);
  o <<= 3;
  o |= p;
  shiftOut32(o);
  delay(dly);
  shiftOut32(0);
}

void poofer_vortex(void)
{
   int i, count;
   
   for(count = 0; count < 10; count++)
   {
     for(i = 0; i < 3; i++)
     {
        poutput(i, 150);
     }
   }
}

void poofer_random(void)
{
   int i, count, dur, p;
   
   for(count = 0; count < 20; count++)
   {
     p = random(3);
     dur = random(300) + 150;
     poutput(p, dur);
   }
}

#define MAX_PATTERNS 6
#define MAX_POOF_PATTERNS 2

// main loop
void loop()
{
  int dly=1000;
  
  // pat1(dly);
  // pat2(dly);
//   cylon(15, dly);
//  rnd(dly);
//    blinkem(250);
//    for(;;)
//    {
//       switch(random(MAX_POOF_PATTERNS))
//       {
///          case 0:
//             poofer_vortex();
//          case 1:
//             poofer_random();
//       }
 //   }  
       
    test_pat(1, 150);
    //for(;;)
    //random_fill();
    for(;;)
    {  
       switch(random(MAX_PATTERNS))
       {
          case 0:     
             test_pat(5, 110);
             break;
          case 1:
             spiral();
             break;
          case 2:
             seizure();
             break;
          case 3:
             line_up_down();
             break;
          case 4:
             line_round();
             break;
          case 5:
             random_fill();
             break;
       }
    }
}




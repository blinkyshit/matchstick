#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

// Bit manipulation macros
#define sbi(a, b) ((a) |= 1 << (b))       //sets bit B in variable A
#define cbi(a, b) ((a) &= ~(1 << (b)))    //clears bit B in variable A
#define tbi(a, b) ((a) ^= 1 << (b))       //toggles bit B in variable A

char morse1[] PROGMEM = ".-.. ..- -. .- .-. ...- .. .-.. .-.. . ";

// set up pins connected to 74HC595 chain
int       clockPin = 8;  // PB0
int       dataPin = 11;  // PB3
int       latchPin = 12; // PB4

volatile uint8_t done = 0;
volatile uint8_t count = 0;

#define MAX_COUNT 16

ISR (TIMER2_OVF_vect)
{
    if (count == MAX_COUNT)
        done = 1;

    count++;
}

uint8_t is_done(void)
{
    uint8_t d;

    cli();
    d = done;
    sei();

    return d;
}

void clear_done(void)
{
    cli();
    done = 0;
    sei();
}

void setup()
{
    // setup output pins
    DDRB |= (1<<PB0) | (1 << PB3) | (1 << PB4) | (1 << PB5);
    cbi(PORTB, 0);
    cbi(PORTB, 3);
    cbi(PORTB, 4);

    // Setup timers
    TCCR2B |= _BV(CS22) | _BV(CS21) | _BV(CS20);
	TCNT2 = 0;
    TIMSK2 |= _BV(TOIE2);
}

// shift 32 bits into74HC595 chain, MSB first
void shift_out(unsigned long data)
{
    int8_t i = 0;

    for (i = 31; i >= 0; i--)
    {
        if (data & (1 << i))
            cbi(PORTB, 3);
        else
            sbi(PORTB, 3);

        sbi(PORTB, 0);
        cbi(PORTB, 0);
    }
    sbi(PORTB, 4);
    cbi(PORTB, 4);
}

#define SHORT_DUR 30 
#define LONG_DUR 90 

void morse()
{
    char *PROGMEM  ptr;
    char          ch;
    unsigned int  delay;
    unsigned int  led;
    unsigned int  i;

    shift_out(0);
    for(i = 0; i < 4; i++)
        _delay_ms(250);

    for(ptr = morse1;; ptr++)
    {
        ch = pgm_read_byte(ptr);
        if (!ch)
            return;

        if (ch == '-')
           delay = LONG_DUR;
        else
           delay = SHORT_DUR;

        if (ch == ' ')
           led = 0;
        else
           led = 1;

        if (led)
            shift_out(0xFFFF);

        for(i = 0; i < delay; i++)
           _delay_ms(10);

        shift_out(0);

        for(i = 0; i < SHORT_DUR; i++)
           _delay_ms(10);
    }

    shift_out(0);
    for(i = 0; i < 4; i++)
        _delay_ms(250);
}

void cylon(int length, int dly)
{
    unsigned long i = 1;
    int       j;

    for (j = 0; j < length; j++)
    {
        shift_out(i);
        i = i << 1;
        _delay_ms(dly);
    }
    for (j = 0; j < length; j++)
    {
        shift_out(i);
        i = i >> 1;
        _delay_ms(dly);
    }
    return;
}

unsigned long crtl(int col, int row)
{
    return (1 << (((unsigned long) col * 4) + (unsigned long) row));
}

void up_down(void)
{
    long      row, col;
    int       count;

    for (count = 0; !is_done(); count++)
    {
        for (col = 0; col < 4; col++)
        {
            for (row = 0; row < 4; row++)
            {
                shift_out(crtl(col, row));
                _delay_ms(75);
            }
            for (row = 3; row >= 0; row--)
            {
                shift_out(crtl(col, row));
                _delay_ms(75);
            }
        }
    }
}

void binary(int dly)
{
    long      col;

    for(;;)
        for (col = 0; col < 65536 && !is_done(); col += 4)
        {
            shift_out(col);
            _delay_ms(dly);
        }
}

void line_up_down(void)
{
    long      row;

    while(!is_done())
    {
        for (row = 0; row < 4; row++)
        {
            shift_out(crtl(0, row) | crtl(1, row) | crtl(2, row) | crtl(3, row));
            _delay_ms(125);
        }
        for (row = 3; row >= 0; row--)
        {
            shift_out(crtl(0, row) | crtl(1, row) | crtl(2, row) | crtl(3, row));
            _delay_ms(125);
        }
    }
}

void line_round(void)
{
    long      col;

    while(!is_done())
    {
        for (col = 0; col < 4; col++)
        {
            shift_out(crtl(col, 0) | crtl(col, 1) | crtl(col, 2) | crtl(col, 3));
            _delay_ms(125);
        }
    }
}

void seizure(void)
{
    int       m;

    for(m = 0; !is_done(); m++)
    {
        shift_out((m % 2) == 1 ? 65535 : 0);
        _delay_ms(75);
    }
}

void spiral(void)
{
    int       i;

    for (i = 0;; i += 5)
    {
        if (i > 15)
        {
            i = i % 16;
            if (is_done())
                break;
        }
        shift_out(1 << i);
        _delay_ms(150);
    }
}

void random_fill(void)
{
    int       i, out = 0, old = 0;

    while(!is_done())
    {
        out = 0;
        for (i = 0; i < 16; i++)
        {
            while (1)
            {
                old = out;
                out |= (1 << (random() % 16));
                if (old != out)
                    break;
            }
            shift_out(out);
            _delay_ms(150);
        }

        out = 0xFFFF;
        for (i = 0; i < 16; i++)
        {
            while (1)
            {
                old = out;
                out &= ~(1 << (random() % 16));
                if (old != out)
                    break;
            }
            shift_out(out);
            _delay_ms(150);
        }
    }
}

void rows_up(void)
{
    long      row;

    while(!is_done())
    {
        for (row = 0; row < 4; row++)
        {
            shift_out(crtl(0, row) | crtl(1, row) | crtl(2, row) | crtl(3, row));
            _delay_ms(125);
        }
    }
}

void row_fill(void)
{
    int       row;
    uint16_t  out = 0;

    while(!is_done())
    {
        for (row = 0; row < 4; row++)
        {
            out |= crtl(0, row) | crtl(1, row) | crtl(2, row) | crtl(3, row);
            shift_out(out);
            _delay_ms(120);
        }
        for (row = 3; row >= 0; row--)
        {
            out &= ~(crtl(0, row) | crtl(1, row) | crtl(2, row) | crtl(3, row));
            shift_out(out);
            _delay_ms(120);
        }
    }
}

void linear(void)
{
    uint8_t i;

    while(!is_done())
        for(i = 0; i < 16; i++)
        {
            shift_out(1 << i);
            _delay_ms(110);
        }
}

void white_noise(void)
{
    uint16_t r;

    while(!is_done())
    {
        r = random();
        shift_out(r);
        _delay_ms(110);
        r = ~r;
        shift_out(r);
        _delay_ms(110);
    }
}

void untz_untz(void)
{
    uint8_t r;

    while(!is_done())
    {
        for(r = 0; r < 3; r++) 
        {
            shift_out(0xFFFF);
            _delay_ms(200);
            shift_out(0);
            _delay_ms(250);
        }
        for(r = 0; r < 3; r++) 
        {
            shift_out(0xFFFF);
            _delay_ms(125);
            shift_out(0);
            _delay_ms(125);
        }
        for(r = 0; r < 3; r++) 
        {
            shift_out(0xFFFF);
            _delay_ms(250);
            shift_out(0);
            _delay_ms(250);
        }
    }
}

void flash_led(void)
{
    uint8_t i;

    for(i = 0; i < 3; i++)
    {
        sbi(PORTB, 5);
        _delay_ms(100); 
        cbi(PORTB, 5);
        _delay_ms(100); 
    }
}

int main(void)
{
    uint8_t i = 0;

    setup();
    flash_led();

    sei();
    if (0)
        for(;;)
        {
            untz_untz();
            clear_done();
        }
    else
        line_up_down();

    up_down();
    for (;;i++)
    {
        //switch (i++)
        switch (random() % 12)
        {
            case 0:
                morse();
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
            case 6:
                up_down();
                break;
            case 7:
                rows_up();
                break;
            case 8:
                row_fill();
                break;
            case 9:
                white_noise();
                break;
            case 10:
                linear();
                break;
            case 11:
                untz_untz();
                break;
        }
        clear_done();
    }
}

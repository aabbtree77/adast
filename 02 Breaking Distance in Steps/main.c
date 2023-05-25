/* atmega16/32, R. G. May 2nd, 2023 */

#define F_CPU 8000000UL  
#include <avr/io.h>
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h> 
#include <avr/interrupt.h>

//macros
#define _SET(type,name,bit)          type ## name  |= _BV(bit)    
#define _CLEAR(type,name,bit)        type ## name  &= ~ _BV(bit)        
#define _TOGGLE(type,name,bit)       type ## name  ^= _BV(bit)    
#define _GET(type,name,bit)          ((type ## name >> bit) &  1)

#define OUTPUT(pin)         _SET(DDR,pin)    
#define INPUT(pin)          _CLEAR(DDR,pin)    
#define HIGH(pin)           _SET(PORT,pin)
#define LOW(pin)            _CLEAR(PORT,pin)    
#define TOGGLE(pin)         _TOGGLE(PORT,pin)    
#define READ(pin)           _GET(PIN,pin)

// IO

// active 1:
#define Seg_ctrl0 A,0	 
#define Seg_ctrl1 A,1
#define Seg_ctrl2 A,2
#define Seg_ctrl3 A,3

// active 0:	
#define Seg_0 A,7	
#define Seg_1 A,6		
#define Seg_2 A,5
#define Seg_3 A,4	 

// active 1:
#define Out_forw C,6
#define Out_back C,5

// active 0:
#define In_cut_inactive D,0
#define In_pos_2225 D,3
#define In_forw D,6
#define In_back D,7
#define In_cancel C,3

#define In_encode_A D,2
#define In_encode_B D,5

#define In_ind0 B,0
#define In_ind1 B,1
#define In_ind2 B,2
#define In_ind3 B,3
#define In_ind4 B,4
#define In_ind5 B,5
#define In_ind6 B,6
#define In_ind7 B,7
#define In_ind8 C,0
#define In_ind9 C,1
#define In_enter0 C,2


//uint8_t is in [0, +255].
//int16_t is in [−32768, +32767].
//int32_t is in [−2147483648, +2147483647].
const int32_t P_MAX_MU = ((int32_t)807000);
const int32_t P_MIN_MU = ((int32_t)28000);
const int32_t P_ST_MU = ((int32_t)223000);

int32_t STEP_MU = ((int32_t)1);
const int32_t OFFSET_MU = ((int32_t)50000);
volatile int32_t P_MU = 0;
volatile int32_t TARGET_MU = 0;
volatile int32_t BREAK_DIST_FORW_MU = 500;
volatile int32_t BREAK_DIST_BACK_MU = 2200;

volatile uint8_t BLINK_MODE = 0;
volatile uint8_t OVF_T1 = 0;
volatile uint8_t SEG_COUNTER = 0;
volatile uint8_t SYMBOL_IDS[] = {0, 0, 0, 0};
volatile uint8_t SYMBOL_TEMP = 0;
volatile uint8_t STEP_COUNT_MODE = 0;
volatile uint8_t FORWARD_MODE = 0;
volatile uint8_t TARGET_DIGIT_IND = 0;
volatile int32_t TARGET_DIGITS[] = {0, 0, 0, 0};

volatile uint8_t STATE_A_NOW = 0;
volatile uint8_t STATE_A_BEFORE = 0;

ISR(TIMER1_OVF_vect)
{
    //Toggle level every 0.5s or so
    OVF_T1 = (OVF_T1 == 1)? 0 : 1;
}

ISR(TIMER2_OVF_vect)
{	
    switch(SEG_COUNTER){	
    case 0:
        //HIGH(Seg_ctrl0);
        if ( (BLINK_MODE == 1) && (TARGET_DIGIT_IND == 0) ) {
            //Toggle ctrl signal
            if (OVF_T1 == 1) { HIGH(Seg_ctrl0); } 
            else { LOW(Seg_ctrl0); }              
        } else {
            HIGH(Seg_ctrl0);  
        }    
        LOW(Seg_ctrl1); 
        LOW(Seg_ctrl2);
        LOW(Seg_ctrl3);      
        SYMBOL_TEMP = SYMBOL_IDS[0];
        break;
    case 1:
        LOW(Seg_ctrl0);
        //HIGH(Seg_ctrl1);
        if ( (BLINK_MODE == 1) && (TARGET_DIGIT_IND == 1) ) {
            //Toggle ctrl signal
            if (OVF_T1 == 1) { HIGH(Seg_ctrl1); } 
            else { LOW(Seg_ctrl1); }              
        } else {
            HIGH(Seg_ctrl1);  
        }   
        LOW(Seg_ctrl2);
        LOW(Seg_ctrl3);        
        SYMBOL_TEMP = SYMBOL_IDS[1];
        break;
    case 2:
        LOW(Seg_ctrl0);
        LOW(Seg_ctrl1);
        //HIGH(Seg_ctrl2);
        if ( (BLINK_MODE == 1) && (TARGET_DIGIT_IND == 2) ) {
            //Toggle ctrl signal
            if (OVF_T1 == 1) { HIGH(Seg_ctrl2); } 
            else { LOW(Seg_ctrl2); }              
        } else {
            HIGH(Seg_ctrl2);  
        }   
        LOW(Seg_ctrl3);        
        SYMBOL_TEMP = SYMBOL_IDS[2];
        break;
    case 3:
        LOW(Seg_ctrl0);
        LOW(Seg_ctrl1);
        LOW(Seg_ctrl2);
        //HIGH(Seg_ctrl3);
        if ( (BLINK_MODE == 1) && (TARGET_DIGIT_IND == 3) ) {
            //Toggle ctrl signal
            if (OVF_T1 == 1) { HIGH(Seg_ctrl3); } 
            else { LOW(Seg_ctrl3); }              
        } else {
            HIGH(Seg_ctrl3);  
        }        
        SYMBOL_TEMP = SYMBOL_IDS[3];
        break;		
    }
    if ((SYMBOL_TEMP & (1<<0))!=0) { HIGH(Seg_0); } else { LOW(Seg_0); }
    if ((SYMBOL_TEMP & (1<<1))!=0) { HIGH(Seg_1); } else { LOW(Seg_1); }
    if ((SYMBOL_TEMP & (1<<2))!=0) { HIGH(Seg_2); } else { LOW(Seg_2); }
    if ((SYMBOL_TEMP & (1<<3))!=0) { HIGH(Seg_3); } else { LOW(Seg_3); }
        
    if ((SEG_COUNTER++) > 3){
        SEG_COUNTER = 0;
    }	
}

void show_position(int32_t value_mu) {
/*
Note: There are only 4 LEDs, 40mu x 9999 roughly 40cm.
*/
    uint8_t i;
    int32_t temp;
    temp = value_mu;
    for (i=0; i<4; i++) {
        SYMBOL_IDS[3-i] = (uint8_t)(temp % 10);
        temp = temp/10;
    }
}

ISR(TIMER0_COMP_vect)
{
    STATE_A_NOW = READ(In_encode_A);
    if (STEP_COUNT_MODE==1) {
        if(STATE_A_NOW < STATE_A_BEFORE) {     
            if(READ(In_encode_B) != STATE_A_NOW) {
                FORWARD_MODE = 1; 
                P_MU -= STEP_MU; 
            } else {
                FORWARD_MODE = 0; 
                P_MU += STEP_MU;
            } 
        }
    }
    STATE_A_BEFORE = STATE_A_NOW;
}

void move_forward(void) {
    HIGH(Out_forw);
    LOW(Out_back);
}

void move_backward(void) {
    LOW(Out_forw);
    HIGH(Out_back);
}

void stop_movement(void) {
    LOW(Out_forw);
    LOW(Out_back);
}

void submove_to_target(int32_t target_val) {
    
    int32_t diff_mu;
    
    diff_mu = target_val - P_MU;
    
    if (diff_mu > 0) {
        while (diff_mu > 0) {
            move_backward();     
            diff_mu = target_val - P_MU;
            show_position(P_MU);
        }
    } else {
        while (diff_mu < 0) {
            move_forward();     
            diff_mu = target_val - P_MU;
            show_position(P_MU);
        }
    }
    stop_movement();
}


//------------------------------------------------------------------------------
int main(void) {
    
    // Timer/Counter0 CTC mode to experiment with the encode detection freq. 
    // Pages 77-80. atmega16.pdf.
    TCCR0 |= (1<<CS00); //no prescaling, leave at 8MHz
    // TCCR0 |= (1<<CS01); //prescaler 8x
    TCCR0 |= (1<<WGM01); //CTC mode
	  TCNT0 = 0; //just in case
	  TIMSK |= (1<<OCIE0); //Timer/Counter0 Output Compare Match Interrupt Enable
    TIMSK |= (1 << TOIE0); //Timer/Counter0 Overflow Interrupt Enable
	  OCR0 = 16;//between 0 to 255, 16 should bring 0.5MHz
    //OCR0 = 80; //100KHz
    
    // TIMER/Counter1 for LED blinking at long (sub-second) intervals.
    // This timer is needed as 255×1024÷8000000MHZ = 0,0326s is not enough.
    // Instead, 65536×64÷8000000MHz = 0.5243s
    // Page 106 atmega16.pdf
    // TCCR1A all zeros - normal mode, overflow at 0xFFFF
    TCCR1B |= ((1 << CS11)|(1<<CS10)); //Prescaler 64.
    TIMSK |= (1 << TOIE1); //Timer/Counter1 Overflow Interrupt Enable
      
    //Timer2 is for display, pages 123-126 atmega16.pdf 
    TCCR2 |= ((1 << CS21)|(1<<CS20)); //Prescaler 32, overflow 0xFF at 8MHz.
    TIMSK |= (1 << TOIE2); //Timer/Counter2 Overflow Interrupt Enable.
    
    _delay_us(50);
    sei(); //globally enable interrupts

    OUTPUT(Seg_ctrl0);
    OUTPUT(Seg_ctrl1);
    OUTPUT(Seg_ctrl2);
    OUTPUT(Seg_ctrl3);
    OUTPUT(Seg_0);
    OUTPUT(Seg_1);
    OUTPUT(Seg_2);
    OUTPUT(Seg_3);
    
    OUTPUT(Out_forw);
    OUTPUT(Out_back);
    _delay_us(50);
    LOW(Out_forw);
    LOW(Out_back);

    INPUT(In_cut_inactive);
    INPUT(In_pos_2225);
    INPUT(In_forw);
    INPUT(In_back);
    INPUT(In_cancel);
    INPUT(In_encode_A);
    INPUT(In_encode_B);
    INPUT(In_enter0);
    
    INPUT(In_ind0);
    INPUT(In_ind1);
    INPUT(In_ind2);
    INPUT(In_ind3);
    INPUT(In_ind4);
    INPUT(In_ind5);
    INPUT(In_ind6);
    INPUT(In_ind7);
    INPUT(In_ind8);
    INPUT(In_ind9);
 
    show_position(1111);
    _delay_ms(2000);
         
    while(1) {
        
        stop_movement();    
        show_position(P_MU);
               
        //Move forward
        while ( (READ(In_forw)==0) && (READ(In_cut_inactive)!=0) ) {
            STEP_COUNT_MODE = 0;
            move_forward();
            show_position(4321);
        }           
        stop_movement(); 
               
        //Move backwards
        while ( (READ(In_back)==0) && (READ(In_cut_inactive)!=0) ) {
            STEP_COUNT_MODE = 0;
            move_backward();
            show_position(1234);
        }        
        stop_movement();
            
        //Move 5000 impulses backwards if "Cancel" is pressed
        if (READ(In_cancel)==0) {
            P_MU = 0;
            STEP_MU = 1;
            STEP_COUNT_MODE = 1;
            submove_to_target(5000);
        }
        
        //Move 5000 impulses forward if "Enter" is pressed
        if (READ(In_enter0)==0) {
            P_MU = 9000;
            STEP_MU = 1;
            STEP_COUNT_MODE = 1;
            submove_to_target(4000);
        } 
             
    }       
    return 0;
}

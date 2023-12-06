/*** atmega16/32, R. G., December 1st, 2023 ***/

#ifndef F_CPU
#define F_CPU 8000000UL
#endif
  
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

//IO

// active 1:
#define Seg_ctrl0 A,0	 
#define Seg_ctrl1 A,1
#define Seg_ctrl2 A,2
#define Seg_ctrl3 A,3

//active 0:	
#define Seg_0 A,7	
#define Seg_1 A,6		
#define Seg_2 A,5
#define Seg_3 A,4	 

//active 1:
#define Out_forw C,7
#define Out_back C,6
#define Out_LED C,5
#define Out_inv D,2

//Active 0:
#define In_cut_inactive D,0
#define In_pos_2225 D,3
#define In_forw D,5
#define In_back D,6
#define In_cancel D,7
//#define In_encode D,2
#define In_encode_A D,1
#define In_encode_B C,4

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

#define In_blue C,3
#define In_yellow D,4
#define In_green C,2

//uint8_t is in [0, +255]
//int16_t is in [−32768, +32767]
//int32_t is in [−2147483648, +2147483647]

const int32_t P_MAX_MU = ((int32_t)800000);
const int32_t P_MIN_MU = ((int32_t)30000);
const int32_t P_ST_MU = ((int32_t)223000);

volatile int32_t BREAK_DIST_FORW_MU = 2400;
volatile int32_t BREAK_DIST_BACK_MU = 100;

const int32_t STEP_MU = ((int32_t)20); //2 AB-states, half the step, 2x resolution  
const int32_t OFFSET_MU = ((int32_t)50000);
//const int32_t OFFSET_MU = ((int32_t)0);
volatile int32_t P_MU = 0;
volatile int32_t TARGET_MU = 0;

volatile uint8_t SEG_COUNTER = 0;
volatile uint8_t SYMBOL_IDS[] = {0, 0, 0, 0};
volatile uint8_t SYMBOL_TEMP = 0;
volatile uint8_t STEP_COUNT_MODE = 0;

volatile uint8_t TARGET_DIGIT_IND = 0;
volatile int32_t TARGET_DIGITS[] = {0, 0, 0, 0};

volatile uint8_t STATE_A_NOW = 0;
volatile uint8_t STATE_A_BEFORE = 0;

volatile uint8_t DUTY_COUNTER = 0;
volatile uint8_t DUTY_MAX_COUNTER = 100;
volatile uint8_t DUTY_HIGH = 20;
volatile uint8_t INVERTER_ACTIVE = 0;

ISR(TIMER2_OVF_vect)
{	
    switch(SEG_COUNTER){	
    case 0:
        HIGH(Seg_ctrl0);
        LOW(Seg_ctrl1); 
        LOW(Seg_ctrl2);
        LOW(Seg_ctrl3);      
        SYMBOL_TEMP = SYMBOL_IDS[0];
        break;
    case 1:
        LOW(Seg_ctrl0);
        HIGH(Seg_ctrl1);
        LOW(Seg_ctrl2);
        LOW(Seg_ctrl3);        
        SYMBOL_TEMP = SYMBOL_IDS[1];
        break;
    case 2:
        LOW(Seg_ctrl0);
        LOW(Seg_ctrl1);
        HIGH(Seg_ctrl2);
        LOW(Seg_ctrl3);        
        SYMBOL_TEMP = SYMBOL_IDS[2];
        break;
    case 3:
        LOW(Seg_ctrl0);
        LOW(Seg_ctrl1);
        LOW(Seg_ctrl2);
        HIGH(Seg_ctrl3);
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


void show_position(int32_t value_mu)
{
    uint8_t i;
    int32_t temp;
    temp = value_mu;
    for (i=0; i<6; i++) {
        //Drop the lowest 2 digits, e.g 8100 for 810000 mu.
        if (i>1) {
            SYMBOL_IDS[5-i] = (uint8_t)(temp % 10);
        }
        temp = temp/10;
    }
}

ISR(TIMER0_OVF_vect)
{
    STATE_A_NOW = READ(In_encode_A);
    if( READ(In_cut_inactive) != 0 ) {
        if (STEP_COUNT_MODE == 1) {
            if(STATE_A_NOW != STATE_A_BEFORE) {     
                if(READ(In_encode_B) == STATE_A_NOW) {
                    //FORWARD_MODE = 1; 
                    P_MU -= STEP_MU; 
                } else {
                    //FORWARD_MODE = 0; 
                    P_MU += STEP_MU;
                } 
            }
        }
    }
    STATE_A_BEFORE = STATE_A_NOW;
}

ISR (TIMER1_COMPA_vect)
{
    if (INVERTER_ACTIVE==1) {
        if (DUTY_COUNTER <= DUTY_HIGH) {
            HIGH(Out_inv);  
        } else {
            LOW(Out_inv); 
        }
    } else {
        LOW(Out_inv);
    }
    
    DUTY_COUNTER++;
    if (DUTY_COUNTER >= DUTY_MAX_COUNTER) {DUTY_COUNTER = 0;}
}

void move_forward(int32_t diff_mu) 
{
    //diff_mu<0 and increases until it hits 0 or above
    
    /*Vary inverter's PWM duty cycle based on the distance
    between the target and current knife position:
    */
    INVERTER_ACTIVE = 1;
    
    if (diff_mu >= -10000) {//1cm
        DUTY_HIGH = 10;
    } else if ( (diff_mu < -10000) && (diff_mu >= -50000) ){
        DUTY_HIGH = 20;
    } else if ( (diff_mu < -50000) && (diff_mu >= -100000) ){
        DUTY_HIGH = 80;
    } else {
        DUTY_HIGH = 80;
    }
    
    LOW(Out_forw);
    HIGH(Out_back);
}

void move_backwards(int32_t diff_mu) 
{
    // diff_mu>0 and decreases until it hits 0 or below
    
    /*Vary inverter's PWM duty cycle based on the distance
    between the target and current knife position:
    */
    INVERTER_ACTIVE = 1;
    
    if (diff_mu <= 10000) {//1cm
        DUTY_HIGH = 10;
    } else if ( (diff_mu > 10000) && (diff_mu <= 50000) ){
        DUTY_HIGH = 20;
    } else if ( (diff_mu > 50000) && (diff_mu <= 100000) ){
        DUTY_HIGH = 80;
    } else {
        DUTY_HIGH = 80;
    }
    
    HIGH(Out_forw);
    LOW(Out_back);
}

void stop_movement(void) 
{
    LOW(Out_forw);
    LOW(Out_back);
    LOW(Out_inv);
    INVERTER_ACTIVE = 0;
}

void set_target(uint8_t ind, uint8_t key)
{
    TARGET_DIGITS[ind] = key;
    TARGET_MU = TARGET_DIGITS[0]*1000+TARGET_DIGITS[1]*100
                +TARGET_DIGITS[2]*10+TARGET_DIGITS[3]*1;
    TARGET_MU *= 100;                 
    show_position(TARGET_MU); 
}

void key_pressed(void) 
{
    uint8_t key = 10;
    uint8_t ind;
    
    while ( READ(In_cancel) == 0) { 
        set_target(0, 0);
        set_target(1, 0);
        set_target(2, 0);
        set_target(3, 0);
        TARGET_DIGIT_IND = 0; 
    }
    
    ind = TARGET_DIGIT_IND; 
        
    while( READ(In_ind0) == 0){ key = 0; set_target(ind, key);}
    while( READ(In_ind1) == 0){ key = 1; set_target(ind, key);}
    while( READ(In_ind2) == 0){ key = 2; set_target(ind, key);}
    while( READ(In_ind3) == 0){ key = 3; set_target(ind, key);}
    while( READ(In_ind4) == 0){ key = 4; set_target(ind, key);}
    while( READ(In_ind5) == 0){ key = 5; set_target(ind, key);}
    while( READ(In_ind6) == 0){ key = 6; set_target(ind, key);}
    while( READ(In_ind7) == 0){ key = 7; set_target(ind, key);}
    while( READ(In_ind8) == 0){ key = 8; set_target(ind, key);}
    while( READ(In_ind9) == 0){ key = 9; set_target(ind, key);}
    
    //If any digit key was pressed move on to the next one, delay debounce
    if (key<10) {
         _delay_ms(10);
        TARGET_DIGIT_IND++; 
        if (TARGET_DIGIT_IND > 3) { TARGET_DIGIT_IND = 0; }    
    }
}

uint8_t target_setup(void) 
{
//set global TARGET_MU to user input.
// TARGET_MU is either target distance or target decrement
    
    uint8_t valid_pos = 0;
    uint8_t valid_decr = 0;
    uint8_t key_last = 0;   
            
    while (1) {
        key_pressed();
        valid_pos = ((TARGET_MU >= P_MIN_MU) && (TARGET_MU <= P_MAX_MU));
        valid_decr = ( ((P_MU-TARGET_MU) >= P_MIN_MU) && 
                        ((P_MU-TARGET_MU) <= P_MAX_MU) );
        if(valid_pos && (READ(In_blue) == 0)) {key_last = 0; break;}
        if(valid_decr && (READ(In_yellow) == 0)) {key_last = 1; break;}   
    }
    
    return key_last;
}

void submove_to_target(int32_t target_val) 
{
    
    int32_t diff_mu;
    
    diff_mu = target_val - P_MU;
    
    if (diff_mu > 0) {
        while (diff_mu > 0) {
            move_backwards(diff_mu);
            if ( (READ(In_cut_inactive)) == 0) {break;}
            //If the sensor is hit, use the opportunity to resync P_MU.
            //if (READ(In_pos_2225) == 0) {
            //    P_MU = P_ST_MU;
            //}        
            diff_mu = target_val - P_MU;
            show_position(P_MU);
        }
    } else {
        while (diff_mu < 0) {
            move_forward(diff_mu);
            if ( (READ(In_cut_inactive)) == 0) {break;}
            //If the sensor is hit, use the opportunity to resync P_MU.
            //if (READ(In_pos_2225) == 0) {
            //    P_MU = P_ST_MU;
            //}        
            diff_mu = target_val - P_MU;
            show_position(P_MU);
        }
    }
    stop_movement();
}

void move_to_target_and_stop(int32_t target_val) {

    int32_t diff_mu;
    diff_mu = target_val - P_MU;
    
    //100mu = 0.1 mm is the smallest allowed target dist difference
    if ( (diff_mu < -90) || (diff_mu > 90) ) {
        if (diff_mu > 0) {
            if( (P_MAX_MU - target_val) > OFFSET_MU ) {
                submove_to_target(target_val + OFFSET_MU - BREAK_DIST_BACK_MU);
                _delay_ms(50);
                submove_to_target(target_val + BREAK_DIST_FORW_MU);          
            } else {
                submove_to_target(target_val - BREAK_DIST_BACK_MU);
            }        
        } else {
            submove_to_target(target_val + BREAK_DIST_FORW_MU);
        }
    }
    //Otherwise exit as diff_mu is too small.
}    

//------------------------------------------------------------------------------
int main(void) {
      
   
    TIMSK |= (1 << TOIE2);
    //Prescaler 32, overflow 0xFF at 8MHz. 255×32÷8000000 = 1.02ms.
    TCCR2 |= ((1 << CS21)|(1<<CS20));
    _delay_us(50);
    
    TIMSK |= (1 << TOIE0);
    //No prescaling, overflow 0xFF at 8MHz, yields roughly 30KHz.
    TCCR0 |= (1<<CS00);
    
    //Interrupt is fired when TCNT1 becomes equal to OCR1A
    TIMSK |= (1 << OCIE1A);
    //No prescaling. 8MHz signal counter reset at OCR1A = 80, leads to 100KHz interrupt.
    //DUTY_MAX_COUNTER = 100 reduces 100KHz to 1KHz PWM. 
    TCCR1B |= (1 << CS10);
    //CTC with OCR1A:
    TCCR1B |= (1 << WGM12);
    TCNT1 = 0;
    OCR1A = 80; //Max value 65535.
    
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
    OUTPUT(Out_LED);
    OUTPUT(Out_inv);
    
    _delay_us(50);
    
    LOW(Out_forw);
    LOW(Out_back);
    LOW(Out_LED);

    INPUT(In_cut_inactive);

    INPUT(In_pos_2225);

    INPUT(In_forw);
    INPUT(In_back);
    INPUT(In_encode_A);
    INPUT(In_encode_B);
    INPUT(In_blue);
    INPUT(In_yellow);
    INPUT(In_cancel);
    INPUT(In_green);
    
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
 
    _delay_ms(100); 
    show_position(0);
    
    uint8_t found_pos2225 = 0;
    uint8_t key_last;
    
    while(1) {  
        
        //Move forward
        while ( (READ(In_forw)==0) && (READ(In_cut_inactive)!=0) ) {
            move_forward(-300000);
            if (found_pos2225 == 1) {
                //If the sensor is hit, use the opportunity to resync P_MU.
                //f (READ(In_pos_2225) == 0) {
                //    P_MU = P_ST_MU;
                //}   
                show_position(P_MU); 
            } else {
                show_position(432100);
            }            
            if ( (READ(In_pos_2225) == 0) && (!(found_pos2225)) ) {
                //First time pos_2555 is located
                P_MU = P_ST_MU;
                STEP_COUNT_MODE = 1;
                found_pos2225 = 1;
                //_delay_ms(1000);
                //stop_movement();
                //_delay_ms(5000); 
                //move_to_target_and_stop(P_ST_MU); 
            } 
        }
        stop_movement(); 
        
        //Move backwards
        while ( (READ(In_back)==0) && (READ(In_cut_inactive)!=0) ) {
            move_backwards(300000);
            if (found_pos2225 == 1) {
                //If the sensor is hit, use the opportunity to resync P_MU.
                //if (READ(In_pos_2225) == 0) {
                //    P_MU = P_ST_MU;
                //}   
                show_position(P_MU); 
            } else {
                show_position(123400);
            }           
            
        }        
        stop_movement(); 
        
        if (found_pos2225 == 1) {
            show_position(P_MU); 
        } else {
            show_position(0);
        }        
        
        //Auto modes
        if ((found_pos2225 == 1) && (READ(In_cancel)==0)) { 
            while (READ(In_cancel)==0) {
                set_target(0, 0);
                set_target(1, 0);
                set_target(2, 0);
                set_target(3, 0);
                TARGET_DIGIT_IND = 0;
            }    
            //target_setup();
            //move_to_target_and_stop(TARGET_MU); 
            
            key_last = target_setup();
            if(READ(In_cut_inactive)!=0) {
                
                if(key_last == 0) {
                
                    LOW(Out_LED);
                    //TARGET_MU is target distance:
                    move_to_target_and_stop(TARGET_MU);
                    
                } else { 
                    _delay_ms(100);
                    while (P_MU > (P_MIN_MU + TARGET_MU)) {
                        
                        HIGH(Out_LED);
                        
                         if ( (READ(In_green)==0) && (READ(In_cut_inactive)!=0) ){
                             move_to_target_and_stop(P_MU - TARGET_MU); //decr
                             _delay_ms(100);
                         }
                         
                         if(READ(In_yellow)==0){ 
                                break; 
                         }
                    }
                    
                }    
                
                LOW(Out_LED);
            
            }    
             
        }
        
    }       
    return 0;
}

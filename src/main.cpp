#include <Arduino.h>
#include <IrNec.h>

const uint8_t SSR_PIN = 1;
const uint8_t LATCH_PIN = 0;
const uint8_t CLOCK_PIN = 3;
const uint8_t DATA_PIN = 4;

const uint8_t DISPLAY_LTU[16] = {0xFF, 0xCF, 0x92, 0x86, 0xCC, 0xA4, 0xA0, 0x8F, 0x80, 0x84, 0x08, 0x00, 0x31, 0x01, 0x30, 0x38};

const uint8_t LEVELS = 15;
uint8_t cycle_number = 1, speed = 0, _last_operation = +1;

void display(uint8_t);

const uint8_t __TOTOAL_ISR_COUNTS = 5;
volatile uint8_t __isr_number = 1;
volatile bool __flag = true;
void __init_T0(void);

void setup(void) {
  nsIrNec::begin();
  pinMode(SSR_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  digitalWrite(SSR_PIN, HIGH);
  display(DISPLAY_LTU[0]);
  __init_T0();
}

void loop(void) {
  nsIrNec::loop();
  if (nsIrNec::dataOut != 0) {
    if ((nsIrNec::dataOut == 0x00F700FF) && (speed != LEVELS)) {
        ++speed;
        _last_operation = +1;
    } else if ((nsIrNec::dataOut == 0x00F7807F) && (speed != 0)) {
        --speed;
        _last_operation = -1;
    } else if ((nsIrNec::dataOut == 0xFFFFFFFF) && ((speed != 0) && (speed != LEVELS))) {
        speed += _last_operation;
    }
    nsIrNec::dataOut = 0;
    display(DISPLAY_LTU[speed]);
  }
}

void display(uint8_t character) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, character);
  digitalWrite(LATCH_PIN, HIGH);
}

void __init_T0(void) {
  TCCR0A = bit(WGM01);
  TCCR0B = bit(CS02);
  TCNT0 = 0;
  OCR0A = 0x7D;
  TIMSK |= bit(OCIE0A);
  sei();
}

ISR (TIMER0_COMPA_vect) {
  if (__isr_number == __TOTOAL_ISR_COUNTS) {
    __isr_number = 1;
    if (__flag) {
      digitalWrite(SSR_PIN, (cycle_number <= speed) ? LOW : HIGH);
      cycle_number += (cycle_number == LEVELS) ? (1 - LEVELS) : 1;
    } else {
      digitalWrite(SSR_PIN, HIGH);
    }
    __flag = !__flag;
  } else {
    ++__isr_number;
  }
}
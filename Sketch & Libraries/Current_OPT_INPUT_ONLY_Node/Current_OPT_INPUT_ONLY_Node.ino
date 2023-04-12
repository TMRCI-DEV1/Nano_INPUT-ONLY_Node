// Import required libraries
#include <Auto485.h>                                // Library for RS485 communication
#include <CMRI.h>                                   // Library for CMRI communication
#include <SPI.h>                                    // Library for SPI communication

const byte CMRI_ADDR = 2;                           // Define CMRI node address
const byte DE_PIN = 2;                              // Define RS485 DE and RE pins on Arduino
#define NOP __asm__ __volatile__("nop")             // "nop" assembly instruction macro

// Define pins for 74HC165
const byte LATCH_165 = 9;

Auto485 bus(DE_PIN);                                // Initialize RS485 bus transceiver
CMRI cmri(CMRI_ADDR, 72, 0, bus); // sets up a modified SMINI. 72 inputs, 0 outputs

byte last_input_state[9];                           // Define variable to store the previous state of inputs

void setup() {
  // Open the RS485 bus at 57600bps
  bus.begin(57600, SERIAL_8N2);

  // 74HC165 setup
  pinMode(LATCH_165, OUTPUT);
  digitalWrite(LATCH_165, HIGH);                    // Initialize Latch High
  SPI.begin ();                                     // Start SPI communication to control 74HC165

  // Initialize last_input_state array with current input state
  digitalWrite(LATCH_165, LOW);                     // Pulse the parallel load latch
  NOP;                                              // Wait while data loads
  NOP;
  digitalWrite(LATCH_165, HIGH);
  for (int i = 0; i < 9; i++) {
    last_input_state[i] = ~(SPI.transfer(0));
  }
}

void loop() {
  // Step 1: Process incoming messages on the CMRI bus
  cmri.process();

  // Step 2: Read the current state of the input pins
  digitalWrite(LATCH_165, LOW);                     // pulse the parallel load latch
  NOP;
  NOP;
  digitalWrite(LATCH_165, HIGH);
  byte currentInputState[9];
  for (int i = 0; i < 9; i++) {
    currentInputState[i] = ~(SPI.transfer(0));
  }

  // Step 3: If the input state has changed, update the CMRI input bytes
  if (memcmp(currentInputState, last_input_state, 9) != 0) {
    memcpy(last_input_state, currentInputState, 9);
    for (int i = 0; i < 9; i++) {
      cmri.set_byte(i, currentInputState[i]);
    }
  }

  // Step 4: Wait for the next iteration
  delay(1);
}

class Expansion // A pair of SIPO shift registers use 3 MCU outputs, expanding outputs to 16
{               // A PISO register requires 1 additional MCU input, expanding inputs to 8
                // Intended for use with CD4094 SIPO and CD4021 PISO registers
                // Works with one port at a time only, determined by pin number given for shift clock
  public:       
    Expansion(uint8_t shiftOutDataPin, uint8_t shiftInDataPin, uint8_t shiftClockPin, uint8_t strobePin);
    void setOutOneVal(uint8_t outOneVal);
    void setOutTwoVal(uint8_t outTwoVal);
    uint8_t getInOneVal();
    boolean runExpansion; // Used as a signal to outside task management that this service wants to run
    void updateExpansion(); // Used to invoke an update, synchronise with timing mechanisms
  
  private:
    uint8_t _shiftByte(uint8_t nextByteOut);
    void _pulseStrobe();
    void _pulseClock();
    uint8_t _outOneVal; // Bits for loading SIPO shift registers
    uint8_t _outTwoVal;
    uint8_t _inOneVal;  // Bits read from PISO shift register
    volatile uint8_t *_out;
    uint8_t _clockMask; // Masks for writing one bit to an eight bit port
    uint8_t _dataOutMask;
    uint8_t _strobeMask;
    uint8_t _port;
};

Expansion::Expansion(uint8_t shiftOutDataPin, uint8_t shiftInDataPin, uint8_t shiftClockPin, uint8_t strobePin)
{ //uses masking and eight bit port writes as with digitalWrite but only creates masks and pointer once
  //determine port and bit mask from given Arduino pin number - allows much faster pin toggling than with digitalWrite
  _clockMask = digitalPinToBitMask(shiftClockPin); //create masks for writing to individual pins
  _dataOutMask = digitalPinToBitMask(shiftOutDataPin);
  _strobeMask = digitalPinToBitMask(strobePin);
  _port = digitalPinToPort(shiftClockPin); // Identify port used - all pins need to be on same port!!
  _out = portOutputRegister(_port); // Initialise pointer to port
}

void Expansion::setOutOneVal(uint8_t outOneVal) { _outOneVal = outOneVal; }

void Expansion::setOutTwoVal(uint8_t outTwoVal) { _outTwoVal = outTwoVal; }

// Using one serial in - parallel out register
void Expansion::updateExpansion() {  _shiftByte(_outOneVal); _pulseStrobe(); runExpansion = false;  } // Write to one register

// Using two serial in - parallel out registers - last bit sent will be in shift reg two LSB
//void Expansion::updateExpansion() { _shiftByte(_outOneVal); _shiftByte(_outTwoVal);  _pulseStrobe(); runExpansion = false; } 

// Using a parallel in - serial out register - done in conjunction with SIPO registers so as not to mess up the serial holding buffers
//void Expansion::updateExpansion() { _pulseStrobe(); _inOneVal =  _shiftByte(_outOneVal); _shiftByte(_outTwoVal);  _pulseStrobe(); runExpansion = false; }

uint8_t Expansion::_shiftByte(uint8_t nextByteOut)
{
  uint8_t nextByteIn = 0;
  for(uint8_t bitCount = 0; bitCount <=7; bitCount++)
  {
    if (nextByteOut>>bitCount & B00000001) *_out |= _dataOutMask; else *_out &= ~_dataOutMask; // Output
    //nextByteIn += digitalRead(_shiftInDataPin); nextByteIn<<1; // Input
    _pulseClock();
  }
  *_out &= ~_dataOutMask; // Clear down data pin
  return nextByteIn;
}

void Expansion::_pulseStrobe()
{      //strobe = B00000001 portB
  *_out |= _strobeMask; *_out &= ~_strobeMask;
}

void Expansion::_pulseClock()
{       //clock = B00000100 portB
  *_out |= _clockMask; *_out &= ~_clockMask; //~ means bitwise NOT of value so B00000100 becomes B11111011
}

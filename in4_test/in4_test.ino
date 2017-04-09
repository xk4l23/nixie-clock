#include <Ticker.h>
#include <SPI.h>

void update_displays(uint8_t *data);

#define LATCH_pin 16
#define OE_pin 2
#define HVEN_pin 15

struct dtime_t {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
} local_time, old_time;

struct settings_t {
  boolean update_display;
} settings;

Ticker ticks;

void setup() {
  //init SPI driver for '595
  Serial.begin(115200);
  Serial.println();
  SPI.begin();
  //latch pin for '595
  pinMode(LATCH_pin, OUTPUT);
  //output enable (low) for '595
  pinMode(OE_pin, OUTPUT);
  //analogWrite(OE_pin, 10);
  digitalWrite(OE_pin, LOW);
  //HVPS enable
  pinMode(HVEN_pin, OUTPUT);
  digitalWrite(HVEN_pin, HIGH);

  ticks.attach_ms(1000, tock);

  //dumy init
  local_time.hours = 17;
  local_time.minutes = 35;
  local_time.seconds = 00;
  boot_animation();
}

void loop() {

  if(settings.update_display) {
    settings.update_display = 0;
    slot();
    //Serial.printf("time is: %2d:%2d\n", local_time.hours, local_time.minutes);
  }
}

void tock() {
  old_time = local_time;
  local_time.seconds += 1;
  if (local_time.seconds > 59) {
    local_time.seconds = 0;
    local_time.minutes += 1;
    settings.update_display = 1;
  }
  if (local_time.minutes > 59) {
    local_time.minutes = 0;
    local_time.hours += 1;
  }
  if (local_time.hours > 23) {
    local_time.hours = 0;
  }
  
}

void boot_animation() {
  uint8_t data[4] = {0};
  uint8_t digit;
  for(digit = 0; digit < 10; digit++) {
    data[0] = data[1] = data[2] = data[3] = digit;
    update_displays(data);
    delay(250);
  }
  data[0] = local_time.hours / 10;
  data[1] = local_time.hours % 10;
  data[2] = local_time.minutes / 10;
  data[3] = local_time.minutes % 10;
  update_displays(data);
}

void slot() {
  
  uint8_t old_values[4] = {0};
  uint8_t new_values[4] = {0};

  //break new/old time into individual digits
  //old time
  old_values[0] = (old_time.hours / 10);
  old_values[1] = (old_time.hours % 10);
  old_values[2] = (old_time.minutes / 10);
  old_values[3] = (old_time.minutes % 10);
  
  //new time
  new_values[0] = (local_time.hours / 10);
  new_values[1] = (local_time.hours % 10);
  new_values[2] = (local_time.minutes / 10);
  new_values[3] = (local_time.minutes % 10);
  
  uint8_t frames[11][4] = {0};
  uint8_t frame_idx = 0;
  uint8_t digit;
  uint8_t value;

  for (digit = 0; digit < 4; digit++) {
    if (new_values[digit] != old_values[digit] ) {
      
      value = new_values[digit];
      for (frame_idx = 0; frame_idx < 11; frame_idx++) {
        frames[frame_idx][digit]=value++;
        if(value > 9) {
          value = 0;
        }
      }
    } else {
      for (frame_idx = 0; frame_idx < 11; frame_idx++) {
        frames[frame_idx][digit] = old_values[digit];
      }
    }
  }

  for (frame_idx = 0; frame_idx < 11; frame_idx++) {
    //Serial.printf("Frame [%2d]: value [%2d:%2d:%2d:%2d]\n", frame_idx, frames[frame_idx][0], frames[frame_idx][1], frames[frame_idx][2], frames[frame_idx][3]);
    update_displays(frames[frame_idx]);
    delay(100);
  }
}

void update_displays(uint8_t *data) {

  //int16_t data[4] = {0};
  //data[0] = 1 << (hours / 10);
  //data[1] = 1 << (hours % 10);
  //data[2] = 1 << (minutes / 10);
  //data[3] = 1 << (minutes % 10);

  //display 4
  SPI.transfer((uint8_t)(((1 << data[3]) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << data[3]) & 0x00FF));
  //display 3
  SPI.transfer((uint8_t)(((1 << data[2]) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << data[2]) & 0x00FF));
  //display 2
  SPI.transfer((uint8_t)(((1 << data[1]) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << data[1]) & 0x00FF));
  //display 1
  SPI.transfer((uint8_t)(((1 << data[0]) & 0xFF00) >> 8));
  SPI.transfer((uint8_t)((1 << data[0]) & 0x00FF));

  //toggle latch pin
  digitalWrite(LATCH_pin, LOW);
  digitalWrite(LATCH_pin, HIGH);
  digitalWrite(LATCH_pin, LOW);
}

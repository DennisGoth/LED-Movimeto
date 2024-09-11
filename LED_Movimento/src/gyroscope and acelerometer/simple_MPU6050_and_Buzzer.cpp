/**
 * This code demonstrates the use of an MPU6050 gyroscope and accelerometer sensor to control the pitch of a buzzer.
 * 
 * The code includes the following functionality:
 * - Initializes the MPU6050 sensor and configures its settings
 * - Defines a 2D array of musical notes that correspond to different octaves and notes
 * - Implements a `pitch()` function that calculates the current octave and note based on the gyroscope readings
 * - Plays the calculated pitch on the buzzer in the `loop()` function
 * - Prints the accelerometer, gyroscope, and temperature data to the serial monitor
 */
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "pitches.h"

// ESP32 pin GPIO18 connected to piezo buzzer
#define BUZZZER_PIN_1  25
#define BUZZZER_PIN_2  26

struct note
{
  int pitch;
  int octave;
  int duration;
};


// defines  Bb Major scale with 6 octaves
int bb_scale[6][8] = {{NOTE_AS1, NOTE_C1, NOTE_D1, NOTE_DS1, NOTE_F1, NOTE_G1, NOTE_A1, SILENCE}, 
                      {NOTE_AS2, NOTE_C2, NOTE_D2, NOTE_DS2, NOTE_F2, NOTE_G2, NOTE_A2, SILENCE}, 
                      {NOTE_AS3, NOTE_C3, NOTE_D3, NOTE_DS3, NOTE_F3, NOTE_G3, NOTE_A3, SILENCE}, 
                      {NOTE_AS4, NOTE_C4, NOTE_D4, NOTE_DS4, NOTE_F4, NOTE_G4, NOTE_A4, SILENCE},
                      {NOTE_AS5, NOTE_C5, NOTE_D5, NOTE_DS5, NOTE_F5, NOTE_G5, NOTE_A5, SILENCE},
                      {NOTE_AS6, NOTE_C6, NOTE_D6, NOTE_DS6, NOTE_F6, NOTE_G6, NOTE_A6, SILENCE}};

int noteDuration[] = {125, 125, 125, 125, 125, 125, 125, 125, 250, 250, 250, 250, 500, 500, 500, 500, 1000, 1000, 1000, 1500};

struct note melodyCurrentNote = {0, 3, 0};
struct note bassCurrentNote = {0, 0, 0};

// MPU6050 sensor object
Adafruit_MPU6050 mpu;

/**
 * @brief Calculates the duration of a note based on accelerometer data.
 *
 * This function takes an accelerometer event as input and calculates the 
 * total acceleration in the x and y directions. Based on the magnitude 
 * of the total acceleration, it returns a duration for a note from a 
 * predefined set of durations.
 *
 * @param a The accelerometer event containing acceleration data.
 * @return The duration of the note based on the total acceleration.
 */
int defineNoteDuration (float totalAcc){
  if (totalAcc > 0.5 and totalAcc < 0.75) return noteDuration[random(18, 19)];
  if (totalAcc > 0.75 and totalAcc < 3) return noteDuration[random(10, 18)];
  return noteDuration[random(0, 10)];
}

/**
 * Calculates the current octave and note based on the gyroscope readings, and returns the corresponding pitch value from the Bb Major scale.
 *
 * The octave value ranges from 0 to 5, and the note value ranges from 0 to 6. These values are updated based on the X and Y gyroscope readings, respectively.
 *
 * @param g The gyroscope event data.
 * @return The pitch value from the Bb Major scale corresponding to the current octave and note.
 */
void defineMelodyNote(float totalAcc, float totalSpin){
  int octave = melodyCurrentNote.octave;
  int pitch = melodyCurrentNote.pitch;

  if (totalAcc < 3) octave -= 1;
  if (totalAcc >= 3) octave += 1;

  if (octave < 0) octave = 5;
  if (octave > 5) octave = 2;
  
  if (totalSpin < 3) pitch -= random(0, 6);
  if (totalSpin > 4) pitch += random(0, 6);

  if (pitch < 0) pitch = abs(pitch);
  while (pitch > 6) pitch -= 3;

  if(totalAcc < 0.5 || totalSpin < 0.5) pitch = 7;

  melodyCurrentNote.pitch = pitch;
  melodyCurrentNote.octave = octave;
  melodyCurrentNote.duration = defineNoteDuration(totalAcc);
}

void defineBassNote(float totalAcc, float totalSpin){
  int harmonics[8][3] = {{2, 4, 6}, {3, 5, 0}, {4, 6, 1}, 
                        {5, 0, 2}, {6, 1, 3}, {0, 2, 4}, 
                        {1, 3, 5}, {7, 7, 7}};
  int octave = bassCurrentNote.octave;
  int pitch = bassCurrentNote.pitch;

  if (totalSpin < 3) octave -= 1;
  if (totalSpin >= 3) octave += 1;

  if (octave < 0) octave = 2;
  if (octave > 5) octave = 0;
  
  bassCurrentNote.pitch = harmonics[melodyCurrentNote.pitch][random(0, 2)];
  bassCurrentNote.octave = octave;
  bassCurrentNote.duration = defineNoteDuration(totalAcc);
}

void playNote(sensors_event_t a, sensors_event_t g){
  float totalAcc = sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y);
  float totalSpin = sqrt(g.gyro.x * g.gyro.x + g.gyro.y * g.gyro.y);
  defineMelodyNote(totalAcc, totalSpin);
  defineBassNote(totalAcc, totalSpin);
  Serial.println(melodyCurrentNote.duration);
  Serial.println(melodyCurrentNote.octave);
  Serial.println(melodyCurrentNote.pitch);

  tone(BUZZZER_PIN_1, bb_scale[melodyCurrentNote.octave][melodyCurrentNote.pitch]);
  tone(BUZZZER_PIN_2, bb_scale[bassCurrentNote.octave][bassCurrentNote.pitch]);
}

/**
 * Configures the MPU6050 sensor with the following settings:
 * - Accelerometer range: ±8g
 * - Gyroscope range: ±500 deg/s
 * - Filter bandwidth: 5 Hz
 * 
 * This function initializes the MPU6050 sensor and checks if it is found. If the sensor is not found, the function will enter an infinite loop.
 */
void setMPUConfigurations(){
  Serial.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

/**
 * Prints the data from the MPU6050 sensor, including the accelerometer, gyroscope, and temperature readings.
 *
 * @param a The accelerometer event data.
 * @param g The gyroscope event data.
 * @param temp The temperature event data.
 */
void printMPUData(sensors_event_t a, sensors_event_t g, sensors_event_t temp){
  Serial.print("AccX:");
  Serial.print(a.acceleration.x);
  Serial.print(",AccY:");
  Serial.print(a.acceleration.y);
  Serial.print(",AccZ:");
  Serial.print(a.acceleration.z);
  Serial.print(",RotX:");
  Serial.print(g.gyro.x * (180 / 3.1415));
  Serial.print(",RotY:");
  Serial.print(g.gyro.y * (180 / 3.1415));
  Serial.print(",RotZ:");
  Serial.print(g.gyro.z * (180 / 3.1415));
  Serial.print(",Temp:");
  Serial.println(temp.temperature);
}


void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  setMPUConfigurations();
  delay(100);
}

void loop() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /*sets a pitch to the buzzer according to the rotation on the gyroscope*/
  playNote(a, g);

  printMPUData(a, g, temp);
  
  delay(melodyCurrentNote.duration);
  noTone(BUZZZER_PIN_1);
  noTone(BUZZZER_PIN_2);
}
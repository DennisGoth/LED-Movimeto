#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include <I2S.h>

// Create an instance of the MPU6050
Adafruit_MPU6050 mpu1;

// Create an instance of the AudioSynthWaveform
const int frequency = 440; // frequency of square wave in Hz
const int amplitude = 10000; // amplitude of square wave
const int sampleRate = 44100; // sample rate in Hz
const int bps = 16;

const int halfWavelength = (sampleRate / frequency); // half wavelength of square wave

short sample = amplitude; // current sample value
int count = 0;

i2s_mode_t mode = ADC_DAC_MODE; // Audio amplifier is needed

void configureMPU(Adafruit_MPU6050 &mpu) {
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10); // wait for serial port to open

    // Initialize I2C communication
    Wire.begin();

    // Initialize MPU6050 #1
    if (!mpu1.begin(0x68)) { 
        Serial.println("Failed to find MPU6050 #1");
        while (1) {
            delay(10);
        }
    }
    Serial.println("MPU6050 #1 found!");

    // Set up the sensor
    configureMPU(mpu1);

    // Initialize Audio library
    if (!I2S.begin(mode, sampleRate, bps)) {
        Serial.println("Failed to initialize I2S!");
        while (1); // do nothing
    }
}

void printMPUData(Adafruit_MPU6050 &mpu, sensors_event_t &a, sensors_event_t &g, sensors_event_t &temp, const char* mpuName) {
    Serial.print(mpuName);
    Serial.print("Accel X: "); Serial.print(a.acceleration.x); Serial.print(", ");
    Serial.print("Accel Y: "); Serial.print(a.acceleration.y); Serial.print(", ");
    Serial.print("Accel Z: "); Serial.print(a.acceleration.z); Serial.print(", ");
    Serial.print("Gyro X: "); Serial.print(g.gyro.x); Serial.print(", ");
    Serial.print("Gyro Y: "); Serial.print(g.gyro.y); Serial.print(", ");
    Serial.print("Gyro Z: "); Serial.print(g.gyro.z); Serial.print(", ");
    Serial.print("Temp: "); Serial.println(temp.temperature);
}

void generateSineWave(float amplitude, float frequency) {
    if (count % halfWavelength == 0 ) {
      // invert the sample every half wavelength count multiple to generate square wave
      sample = -1 * sample;
    }

    if(mode == I2S_PHILIPS_MODE || mode == ADC_DAC_MODE){ // write the same sample twice, once for Right and once for Left channel
      I2S.write(sample); // Right channel
      I2S.write(sample); // Left channel
    }else if(mode == I2S_RIGHT_JUSTIFIED_MODE || mode == I2S_LEFT_JUSTIFIED_MODE){
      // write the same only once - it will be automatically copied to the other channel
      I2S.write(sample);
    }

    // increment the counter for the next sample
    count++;
}

void loop() {
    sensors_event_t a1, g1, temp1;

    // Get new sensor events with the readings
    mpu1.getEvent(&a1, &g1, &temp1);

    // Print out the values
    printMPUData(mpu1, a1, g1, temp1, "MPU6050 #1: ");

    // Generate a sine wave with amplitude and frequency based on sensor data
    float amplitude = map(a1.acceleration.x, -2, 2, 0, 255); // Example mapping
    float frequency = map(g1.gyro.x, -250, 250, 20, 2000); // Example mapping
    generateSineWave(amplitude, frequency);
}

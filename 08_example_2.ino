// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // Sonar sensor TRIGGER
#define PIN_ECHO 13   // Sonar sensor ECHO

// Configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25      // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)
#define TIMEOUT ((INTERVAL / 2) * 1000.0) // max echo waiting time (usec)
#define SCALE (0.001 * 0.5 * SND_VEL) // convert duration to distance

unsigned long last_sampling_time = 0; // msec

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);   // sonar ECHO
  digitalWrite(PIN_TRIG, LOW);
  Serial.begin(57600);
}

void loop() {
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  float distance = USS_measure(PIN_TRIG, PIN_ECHO);

  int led_pwm = 0; // default: LED OFF (최소 밝기)

  if ((distance == 0.0) || (distance > _DIST_MAX) || (distance < _DIST_MIN)) {
    led_pwm = 0;
  } else {
    // 200mm에서 255, 100mm/300mm에서 0, 150/250mm에서 127이 되도록 비례식 적용
    if (distance <= 200.0)
      led_pwm = map((int)distance, 100, 200, 0, 255);
    else
      led_pwm = map((int)distance, 200, 300, 255, 0);
    // 100~200mm: 밝기 0→255, 200~300mm: 밝기 255→0
  }

  analogWrite(PIN_LED, led_pwm);

  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(",distance:");  Serial.print(distance);
  Serial.print(",Max:");       Serial.print(_DIST_MAX);
  Serial.print(",led_pwm:");   Serial.print(led_pwm);
  Serial.println();
  unsigned long duration = pulseIn(PIN_ECHO, HIGH, TIMEOUT);
  Serial.print("Duration: ");
  Serial.println(duration);

  last_sampling_time += INTERVAL;
}

// 초음파 측정 함수
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // mm
}

#include <Servo.h>

#define PIN_IR    A0      
#define PIN_LED   9
#define PIN_SERVO 10

#define _DUTY_MIN 0     
#define _DUTY_NEU 1480  
#define _DUTY_MAX 2960  

#define _DIST_MIN 100.0   
#define _DIST_MAX 250.0   

#define EMA_ALPHA 0.2     

#define LOOP_INTERVAL 20  

Servo myservo;
unsigned long last_loop_time;   

float dist_ema = _DIST_MIN;  

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  
  myservo.attach(PIN_SERVO);  
  myservo.writeMicroseconds(_DUTY_NEU); 
  
  Serial.begin(1000000);    
}

void loop()
{
  unsigned long time_curr = millis();
  int duty;
  float a_value, dist_raw;

  if (time_curr < (last_loop_time + LOOP_INTERVAL))
    return;
  last_loop_time = time_curr; 

  a_value = analogRead(PIN_IR);
  
  dist_raw = ((6762.0 / (a_value - 9.0)) - 4.0) * 10.0 - 60.0;

  if (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX) {
    digitalWrite(PIN_LED, HIGH);
  } else {
    digitalWrite(PIN_LED, LOW);
    
    if (dist_raw < _DIST_MIN) {
      dist_raw = _DIST_MIN;
    } else if (dist_raw > _DIST_MAX) {
      dist_raw = _DIST_MAX;
    }
  }

  dist_ema = (EMA_ALPHA * dist_raw) + (1.0 - EMA_ALPHA) * dist_ema;
      
  duty = (int)( (dist_ema - _DIST_MIN) * (_DUTY_MAX - _DUTY_MIN) / (_DIST_MAX - _DIST_MIN) + _DUTY_MIN );
  
  myservo.writeMicroseconds(duty);

  Serial.print("_DUTY_MIN:");  Serial.print(_DUTY_MIN);
  Serial.print(",_DIST_MIN:"); Serial.print(_DIST_MIN);
  Serial.print(",IR:");        Serial.print(a_value);
  Serial.print(",dist_raw:");  Serial.print(dist_raw);
  Serial.print(",ema:");       Serial.print(dist_ema);
  Serial.print(",servo:");    Serial.print(duty);
  Serial.print(",_DIST_MAX:"); Serial.print(_DIST_MAX);
  Serial.print(",_DUTY_MAX:"); Serial.print(_DUTY_MAX);
  Serial.println("");
}

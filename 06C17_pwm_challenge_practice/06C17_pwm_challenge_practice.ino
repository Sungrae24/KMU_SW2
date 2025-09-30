const int ledPin = 7;
int pwm_period = 1000; // 기본값: 1ms
int pwm_duty = 50;    // 기본값: 50%
void set_period(int period) {
  if (period < 100) period = 100;
  if (period > 10000) period = 10000;
  pwm_period = period;
}

void set_duty(int duty) {
  if (duty < 0) duty = 0;
  if (duty > 100) duty = 100;
  pwm_duty = duty;
}

void setup() {
  pinMode(ledPin, OUTPUT);
  set_period(100); // 0.1ms
}
//period를 10ms로 설정하고 싶으면 set_period(10000), 1ms로 설정하고 싶으면 set_period(1000), 0.1ms로 설정하고 싶으면 set_period(100)

void loop() {
  // 밝기 증가 (0→100)
  for (int duty = 0; duty <= 100; duty++) {
    set_duty(duty);
    int on_time = pwm_period * pwm_duty / 100;
    int off_time = pwm_period - on_time;
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(on_time);
    digitalWrite(ledPin, LOW);
    delayMicroseconds(off_time);
  }
  // 밝기 감소 (100→0)
  for (int duty = 100; duty >= 0; duty--) { 
    set_duty(duty);
    int on_time = pwm_period * pwm_duty / 100;
    int off_time = pwm_period - on_time;
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(on_time);
    digitalWrite(ledPin, LOW);
    delayMicroseconds(off_time);
  }
}

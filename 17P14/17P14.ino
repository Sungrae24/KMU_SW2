#include <Servo.h>

// -------------------- 핀/상수 --------------------
static const uint8_t  PIN_IR    = A0;
static const uint8_t  PIN_LED   = 9;
static const uint8_t  PIN_SERVO = 10;

static const uint32_t BAUD        = 1000000; // 플로터와 동일 설정
static const uint32_t INTERVAL_MS = 35;      // 30~40ms 권장

static const float DIST_MIN_MM = 100.0f; // 10 cm
static const float DIST_MAX_MM = 250.0f; // 25 cm

static const float ALPHA = 0.15f;        // 0.12~0.2 권장
static const int   MAX_STEP_DEG = 3;     // 루프당 최대 변화(도)

static const int SAFE_ANGLE = 90;
static const unsigned long RETURN_DELAY_MS = 800;

// -------------------- 상태 --------------------
Servo myServo;

float dist_raw_mm = NAN;
float dist_ema_mm = NAN;

int last_servo_deg = SAFE_ANGLE;
unsigned long last_update_ms = 0;
unsigned long out_of_range_since = 0;

// -------------------- 유틸 --------------------
float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// 주어진 경험식: (6762/(a-9)-4)*10 - 60 [mm]
float a0_to_dist_mm(int a) {
  if (a <= 10) a = 10; // 분모 보호
  return (6762.0f / (float(a) - 9.0f) - 4.0f) * 10.0f - 60.0f;
}

bool in_range_mm(float d) {
  return (d >= DIST_MIN_MM && d <= DIST_MAX_MM);
}

// 100~250mm -> 0~180deg (map() 미사용)
int dist_to_angle_deg(float d_mm) {
  float d = clampf(d_mm, DIST_MIN_MM, DIST_MAX_MM);
  float angle = (d - DIST_MIN_MM) * (180.0f / (DIST_MAX_MM - DIST_MIN_MM));
  int ang = (int)(angle + 0.5f);
  if (ang < 0)   ang = 0;
  if (ang > 180) ang = 180;
  return ang;
}

int step_limit(int target, int current, int max_step) {
  int diff = target - current;
  if (diff >  max_step) return current + max_step;
  if (diff < -max_step) return current - max_step;
  return target;
}

// -------------------- 설정 --------------------
void setup() {
  Serial.begin(BAUD);
  pinMode(PIN_LED, OUTPUT);
  myServo.attach(PIN_SERVO);
  myServo.write(SAFE_ANGLE);
  last_servo_deg = SAFE_ANGLE;
}

// -------------------- 루프 --------------------
void loop() {
  unsigned long now = millis();
  if (now - last_update_ms < INTERVAL_MS) return;
  last_update_ms = now;

  // 1) 샘플링
  int a = analogRead(PIN_IR);

  // 2) 거리 변환
  dist_raw_mm = a0_to_dist_mm(a);

  // 3) 범위 필터 + EMA
  bool ok = in_range_mm(dist_raw_mm);

  if (ok) {
    // EMA 업데이트
    if (isnan(dist_ema_mm)) dist_ema_mm = dist_raw_mm;
    else dist_ema_mm = ALPHA * dist_raw_mm + (1.0f - ALPHA) * dist_ema_mm;

    digitalWrite(PIN_LED, HIGH);
    out_of_range_since = 0;

    // 목표각 산출(EMA 기준) + 각속도 제한
    int target_deg = dist_to_angle_deg(dist_ema_mm);
    int smooth_deg = step_limit(target_deg, last_servo_deg, MAX_STEP_DEG);
    myServo.write(smooth_deg);
    last_servo_deg = smooth_deg;

  } else {
    // 범위 밖: LED OFF, 마지막 유효 각 유지
    digitalWrite(PIN_LED, LOW);

    // 일정 시간 경과 후 안전각으로 서서히 복귀
    if (out_of_range_since == 0) out_of_range_since = now;
    int target_deg = (now - out_of_range_since > RETURN_DELAY_MS) ? SAFE_ANGLE : last_servo_deg;

    int smooth_deg = step_limit(target_deg, last_servo_deg, MAX_STEP_DEG);
    myServo.write(smooth_deg);
    last_servo_deg = smooth_deg;
  }

  // 4) 시리얼 플로터용 출력(항상 한 줄)
  // 포맷 예: IR:123,dist:180,dist_ema:175,servo:90
  Serial.print("IR:");        Serial.print(a);
  Serial.print(",dist:");     Serial.print((int)dist_raw_mm);
  Serial.print(",dist_ema:"); Serial.print(isnan(dist_ema_mm) ? -1 : (int)dist_ema_mm);
  Serial.print(",servo:");    Serial.println(last_servo_deg);
}

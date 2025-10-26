#include <Servo.h>

// 핀 정의
#define PIN_LED 9
#define PIN_TRIG 12
#define PIN_ECHO 13
#define PIN_SERVO 10

// 거리 범위 설정 (mm 단위)
#define _DIST_MIN 180.0  // 최소 거리 18cm
#define _DIST_MAX 360.0  // 최대 거리 36cm

#define _DIST_ALPHA 0.7

// 서보 각도 범위
#define SERVO_MIN 0
#define SERVO_MAX 180

// 전역 변수
Servo myservo;
float dist_ema = 0.0;  // EMA 필터 값
bool isFirst = true;   // 첫 측정 여부

void setup() {
  // 시리얼 통신 초기화
  Serial.begin(57600);
  
  // 핀 모드 설정
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_LED, LOW);
  
  // 서보 모터 초기화
  myservo.attach(PIN_SERVO);
  myservo.write(0);
  
  delay(1000);
}

void loop() {
  // 초음파 센서로 거리 측정
  float dist_raw = measureDistance();
  
  // 범위 필터 적용 (18cm ~ 36cm)
  float filtered_dist = dist_raw;
  
  if (dist_raw < _DIST_MIN || dist_raw > _DIST_MAX) {
    // 측정 범위 밖: LED 끄기, 마지막 유효값 유지
    digitalWrite(PIN_LED, LOW);
    filtered_dist = dist_ema;  // 이전 EMA 값 유지
  } else {
    // 측정 범위 내: LED 켜기
    digitalWrite(PIN_LED, HIGH);
  }
  
  // EMA 필터 적용
  if (isFirst) {
    // 첫 측정값은 그대로 사용
    dist_ema = filtered_dist;
    isFirst = false;
  } else {
    // EMA 공식: EMA_k = α * d_k + (1-α) * EMA_(k-1)
    dist_ema = _DIST_ALPHA * filtered_dist + (1.0 - _DIST_ALPHA) * dist_ema;
  }
  
  // 거리를 서보 각도로 변환
  int servo_angle = distanceToAngle(dist_ema);
  
  // 서보 모터 제어
  myservo.write(servo_angle);
  
  // 시리얼 플로터용 출력 형식
  Serial.print("Min:");
  Serial.print(_DIST_MIN);
  Serial.print(",dist:");
  Serial.print(dist_raw);
  Serial.print(",ema:");
  Serial.print(dist_ema);
  Serial.print(",servo:");
  Serial.print(myservo.read());
  Serial.print(",Max:");
  Serial.print(_DIST_MAX);
  Serial.println();
  
  delay(50);  // 측정 주기 50ms
}

// 초음파 센서로 거리 측정 함수 (mm 단위)
float measureDistance() {
  // 트리거 핀 클린 신호
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  
  // 10us 펄스 발생
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  // Echo 핀에서 펄스 시간 측정 (마이크로초)
  float duration = pulseIn(PIN_ECHO, HIGH, 25000);
  
  // 거리 계산 (mm 단위)
  // 거리(mm) = duration(us) * 0.343(mm/us) / 2
  float distance = duration * 0.343 / 2.0;
  
  // 측정 실패 시 (0이면) 이전 값 반환
  if (distance == 0) {
    distance = dist_ema;
  }
  
  return distance;
}

// 거리를 서보 각도로 변환하는 함수
int distanceToAngle(float distance) {
  int angle;
  
  if (distance <= _DIST_MIN) {
    // 18cm 이하: 0도
    angle = SERVO_MIN;
  } else if (distance >= _DIST_MAX) {
    // 36cm 이상: 180도
    angle = SERVO_MAX;
  } else {
    // 18cm ~ 36cm 사이: 선형 매핑
    // angle = (distance - 180) / (360 - 180) * 180
    angle = (int)((distance - _DIST_MIN) / (_DIST_MAX - _DIST_MIN) * SERVO_MAX);
  }
  
  // 각도 범위 제한
  angle = constrain(angle, SERVO_MIN, SERVO_MAX);
  
  return angle;
}

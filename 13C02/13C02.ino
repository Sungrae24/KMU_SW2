#include <Servo.h>

Servo myServo;

// 서보 설정
const int SERVO_PIN = 9;
float _SERVO_SPEED = 3.0;  // 초기 속도 (degrees/second)

// 서보 위치 제어 변수
float currentAngle = 0.0;
float targetAngle = 0.0;
unsigned long previousMillis = 0;
int phase = 0;  // 0: 1단계, 1: 2단계, 2: 완료

void setup() {
  Serial.begin(9600);
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // 초기 위치 0도
  currentAngle = 0.0;
  
  // 첫 번째 단계 시작 (0도 → 180도)
  _SERVO_SPEED = 3.0;  // 3 degrees/second
  targetAngle = 180.0;
  phase = 0;
  
  Serial.println("Phase 1: Moving 180 degrees in 60 seconds (3 deg/sec)");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // 업데이트 주기 계산 (밀리초 단위)
  // _SERVO_SPEED가 degrees/second이므로, 1밀리초당 이동할 각도는:
  float updateInterval = 20.0;  // 20ms마다 업데이트
  
  if (currentMillis - previousMillis >= updateInterval) {
    previousMillis = currentMillis;
    
    // 현재 속도로 20ms 동안 이동할 각도 계산
    float angleStep = _SERVO_SPEED * (updateInterval / 1000.0);
    
    // Phase 0: 0도 → 180도 (60초)
    if (phase == 0) {
      if (currentAngle < targetAngle) {
        currentAngle += angleStep;
        if (currentAngle > targetAngle) {
          currentAngle = targetAngle;
        }
        myServo.write((int)currentAngle);
      } else {
        // 1단계 완료, 2단계 시작
        Serial.println("Phase 1 complete!");
        Serial.println("Phase 2: Moving 90 degrees in 300 seconds (0.3 deg/sec)");
        
        _SERVO_SPEED = 0.3;  // 0.3 degrees/second
        targetAngle = 90.0;  // 180도 → 90도 (역방향 90도 이동)
        phase = 1;
      }
    }
    // Phase 1: 180도 → 90도 (300초)
    else if (phase == 1) {
      if (currentAngle > targetAngle) {
        currentAngle -= angleStep;
        if (currentAngle < targetAngle) {
          currentAngle = targetAngle;
        }
        myServo.write((int)currentAngle);
      } else {
        // 2단계 완료
        Serial.println("Phase 2 complete! All movements finished.");
        phase = 2;
      }
    }
    
    // 디버깅용 출력 (선택사항)
    if (phase < 2) {
      Serial.print("Current Angle: ");
      Serial.println(currentAngle);
    }
  }
}

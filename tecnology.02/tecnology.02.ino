#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

RoboEyes<Adafruit_SSD1306> roboEyes(display);

#define TOUCH_PIN 6

// Default sitting servo angles
#define S1_SIT 0
#define S2_SIT 0

// Happy mode servo angles
#define S1_HAPPY 0
#define S2_HAPPY 0

// Walking gait delay
#define WALK_DELAY 400
#define SIT_DELAY 600

Servo s1;
Servo s2;

bool wasTouched = false;
unsigned long touchStart = 0;
bool walkingMode = false;
bool happyMode = false;
unsigned long lastMove = 0;
int servoState = 0;

void setup() {
  Serial.begin(9600);
  pinMode(TOUCH_PIN, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setCuriosity(ON);

  s1.attach(2);
  s2.attach(3);

  // Initial sitting pose
  s1.write(S1_SIT);
  s2.write(S2_SIT);
}

void loop() {
  bool isTouched = digitalRead(TOUCH_PIN) == HIGH;

  if (isTouched) {
    if (!wasTouched) {
      touchStart = millis();
      walkingMode = false;
      happyMode = false;
    }

    unsigned long holdTime = millis() - touchStart;

    if (holdTime >= 3000 && !walkingMode) {
      // Enter walking mode
      walkingMode = true;
      roboEyes.setIdleMode(OFF);
      roboEyes.setCuriosity(OFF);
      roboEyes.setPosition(N);
      roboEyes.setMood(DEFAULT);
      Serial.println("WALKING MODE!");
    } else if (holdTime < 2000 && !walkingMode && !happyMode) {
      // Enter happy mode
      happyMode = true;
      roboEyes.setIdleMode(OFF);
      roboEyes.setCuriosity(OFF);
      roboEyes.setPosition(N);
      roboEyes.setMood(HAPPY);
      roboEyes.anim_laugh();

      s1.write(S1_HAPPY);
      s2.write(S2_HAPPY);
    }

    // Walking servo gait
    if (walkingMode && millis() - lastMove >= WALK_DELAY) {
      lastMove = millis();
      if (servoState == 0) {
        s1.write(90);
        s2.write(0);
        servoState = 1;
      } else {
        s1.write(180);
        s2.write(90);
        servoState = 0;
      }
    }

  } else {
    if (wasTouched) {
      walkingMode = false;
      happyMode = false;
      roboEyes.setPosition(DEFAULT);
      roboEyes.setMood(DEFAULT);
      roboEyes.setIdleMode(ON, 2, 2);
      roboEyes.setCuriosity(ON);
      Serial.println("RESET to DEFAULT");
    }

    // Default sitting pose
    if (millis() - lastMove >= SIT_DELAY) {
      lastMove = millis();
      s1.write(S1_SIT);
      s2.write(S2_SIT);
    }
  }

  wasTouched = isTouched;
  roboEyes.update();
}

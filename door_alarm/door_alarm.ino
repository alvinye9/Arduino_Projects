#define TRIG_PIN 9
#define ECHO_PIN 10
#define BUZZER_PIN 6
#define RESET_BUTTON_PIN 7  // Define reset button pin

long duration;
int distance;
int lastDistance = 0;
const int threshold = 50; // Minimum change in distance to trigger the buzzer

void setup() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP); // Use internal pull-up resistor
    Serial.begin(9600);
}

void loop() {
    if (digitalRead(RESET_BUTTON_PIN) == LOW) { 
        Serial.println("Reset Button Pressed! Stopping Buzzer.");
        digitalWrite(BUZZER_PIN, LOW);
        lastDistance = distance; 
        return;
    }

    // Trigger ultrasonic sensor
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = duration * 0.034 / 2; // Convert to cm
    
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // Check if distance change is significant
    if (abs(distance - lastDistance) > threshold && distance < 100) {
        Serial.println("Beeping!");
        for (int i = 0; i < 10; i++) {
            if (digitalRead(RESET_BUTTON_PIN) == LOW) { 
                Serial.println("Reset Button Pressed! Stopping Buzzer.");
                digitalWrite(BUZZER_PIN, LOW);
                lastDistance = distance;
                return; 
            }
            digitalWrite(BUZZER_PIN, HIGH);
            delay(100);
            digitalWrite(BUZZER_PIN, LOW);
            delay(100);
        }
    }
    lastDistance = distance;  

    delay(100);
}

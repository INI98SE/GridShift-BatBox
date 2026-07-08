/**
 * GridShift: BatBox Core Engine (v1.0)
 * Розроблено для мікроконтролера ESP32-C3 Super Mini.
 * Симуляція енергомережі IndustrialCraft 2 (Minecraft) в реальному житті.
 */

#include <Wire.h>

// --- Налаштування ігрових констант ---
const int MAX_CAPACITY = 40000;       // Максимальна ємність BatBox (еЕ)
float currentEnergy = 25000.0;        // Поточний заряд (стартує з 25к для тесту)

// --- Змінні для фільтрації шумів датчика ---
float averageInput = 0.0;
const float filterCoefficient = 0.15; // Швидкість реакції лічильника (EMA фільтр)

// --- Тестові прапорці (замінити на реальні вимикачі) ---
bool outputConnected = true; 

void setup() {
  // Насильно ініціалізуємо шину I2C на пінах для ESP32-C3 Super Mini
  // GPIO 8 = SDA, GPIO 9 = SCL
  Wire.begin(8, 9); 
  
  // Запуск монітора порту на сучасній швидкості для ESP32
  Serial.begin(115200);
  while (!Serial); 
  
  Serial.println("=========================================");
  Serial.println("  GridShift: BatBox v1.0 Initialized  ");
  Serial.println("=========================================");
}

void loop() {
  // =========================================================================
  // ТИМЧАСОВА ЕМУЛЯЦІЯ ДАТЧИКА INA219 
  // (Коли підключиш реальний INA, тут будуть функції: ina219.getBusVoltage_V() та ін.)
  float mock_U_volts = 5.0; 
  float mock_I_mA = 3205.0; // 3.205 Ампера. Потужність = 5.0 * 3.205 = 16.025 Ват
  // =========================================================================

  // 1. Розрахунок фізичної потужності у Ватах (P = U * I)
  float real_watts = mock_U_volts * (mock_I_mA / 1000.0);

  // 2. Конвертація у віртуальні ігрові EU/t (Приймаємо базове співвідношення: 1 Ват = 1 EU/t)
  float current_instant_EUt = real_watts;

  // 3. Інтелектуальний цифровий фільтр: якщо значення дуже близьке до цілого числа,
  // ми примусово округлюємо його. Це прибирає мікроколивання датчика струму на екрані,
  // повертаючи ідеальні майнкрафтівські числа (наприклад: 2.00 або 16.00 EU/t).
  if (abs(current_instant_EUt - round(current_instant_EUt)) < 0.08) {
    current_instant_EUt = round(current_instant_EUt);
  }

  // 4. Розрахунок рухомого середнього (Емуляція лічильника середнього значення за тики)
  averageInput = (current_instant_EUt * filterCoefficient) + (averageInput * (1.0 - filterCoefficient));

  // 5. Перевірка на перевищення вхідної напруги LV (Низька напруга в IC2 — до 32 EU/t)
  if (current_instant_EUt > 32.5) { 
    triggerAlertMode(); // Спрацьовує захист від високої напруги МФЕ
  } else {
    // Енергообмін за 1 ігровий такт (1 тік = 50 мілісекунд)
    currentEnergy += current_instant_EUt; 

    // Обмеження буфера енергохранилища
    if (currentEnergy > MAX_CAPACITY) currentEnergy = MAX_CAPACITY;
    if (currentEnergy < 0) currentEnergy = 0;

    // Вивід поточної статистики в консоль (наступний крок — перенаправлення на OLED)
    printNetworkStatus(current_instant_EUt);
  }

  // Затримка рівно в 1 ігровий тік (20 тиків на секунду = 50 мілісекунд)
  delay(50); 
}

// Функція виводу стану енергомережі
void printNetworkStatus(float instantEUt) {
  Serial.print("GridShift | Вхід: ");
  Serial.print(instantEUt, 2);
  Serial.print(" EU/t | Середнє: ");
  Serial.print(averageInput, 2);
  Serial.print(" EU/t | Буфер: ");
  Serial.print((int)currentEnergy);
  Serial.println(" / 40000 EU");
}

// Аварійний режим при перевантаженні мережі
void triggerAlertMode() {
  Serial.println("!!! [CRITICAL ERROR] VOLTAGE OVERLOAD! NEED LV-TRANSFORMER !!!");
  // Тут буде логіка відсікання живлення реле/мосфетом та увімкнення сирени
  delay(500); // Сповільнюємо цикл під час аварії
}

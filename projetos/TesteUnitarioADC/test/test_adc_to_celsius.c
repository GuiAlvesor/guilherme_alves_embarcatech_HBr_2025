#include "../drivers/adc_to_celsius.h"
#include "../Unity/unity.h"
#include "pico/stdlib.h"

//float adc_to_celsius(uint16_t adc_val);
void setUp(void);
void tearDown(void);
void test_adc_to_celsius_with_known_value(void);

void setUp(void) {}

void tearDown(void) {}

void test_adc_to_celsius_with_known_value(void) {
    uint16_t adc_val = 876;
    float expected_temp = 27.0f;
    float temp = adc_to_celsius(adc_val);

    TEST_ASSERT_FLOAT_WITHIN(0.5f, expected_temp, temp);
}

int main(void) {
    stdio_init_all();
<<<<<<< HEAD

    // Aguarda a USB ficar disponível
    sleep_ms(2000);

    UNITY_BEGIN();
    RUN_TEST(test_adc_to_celsius_with_known_value);
    UNITY_END();

    while (true) {
        tight_loop_contents(); // mantém a USB ativa
    }

    return 0;
}
=======
    sleep_ms(2000);
    UNITY_BEGIN();
    RUN_TEST(test_adc_to_celsius_with_known_value);
    UNITY_END();
    while(true) {
        tight_loop_contents();
    }
}
>>>>>>> 3d64982fdf3e6747d2605933fa652b08bf641322

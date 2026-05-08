#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS 3000
// Define o LED usando Device Tree
#define LED0_NODE DT_ALIAS(led0) //verde
#define LED1_NODE DT_ALIAS(led1) //azul
#define LED2_NODE DT_ALIAS(led2) //vermelho

// Verifica se o LED está definido no Device Tree
#if DT_NODE_HAS_STATUS(LED0_NODE, okay) && DT_NODE_HAS_STATUS(LED1_NODE, okay) && DT_NODE_HAS_STATUS(LED2_NODE, okay)
static const struct gpio_dt_spec led_g = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led_b = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
#else
#error "Unsupported board: some led devicetree alias is not defined"
#endif


void liga(char cor) {
    // Considerando que em um semaforo apenas uma cor liga por vez, apagaremos as outras cores nesta funcao tambem
    if (cor == 'g') {
        gpio_pin_set_dt(&led_g, 1); //liga verde
        gpio_pin_set_dt(&led_r, 0);
        gpio_pin_set_dt(&led_b, 0);
        return;
    }

    if (cor == 'r') {
        gpio_pin_set_dt(&led_g, 0);
        gpio_pin_set_dt(&led_r, 1); //liga vermelho
        gpio_pin_set_dt(&led_b, 0);
        return;
    }

    if (cor == 'y') {
        gpio_pin_set_dt(&led_g, 1); //liga amarelo
        gpio_pin_set_dt(&led_r, 1); //liga amarelo
        gpio_pin_set_dt(&led_b, 0);
        return;
    }

}

void semaforo(int *estado) {
// máquina de estados do semáforo

    if (*estado == 0) {
    //desliga todos os leds
        gpio_pin_set_dt(&led_g, 0);
        gpio_pin_set_dt(&led_r, 0);
        gpio_pin_set_dt(&led_b, 0);
        *estado = 1; // PARE 
        k_msleep(SLEEP_TIME_MS);
        return;
    }

    if (*estado == 1) {
        // vermelho
        liga('r');
        printk("PARE\n");
        *estado = 2; // PROSSIGA
        k_msleep(SLEEP_TIME_MS);
        return;
    }

    if (*estado == 2) {
        // verde
        liga('g');
        printk("PROSSIGA\n");
        *estado = 3; // ATENCAO
        k_msleep(SLEEP_TIME_MS);
        return;
    }

    if (*estado == 3) {
        // amarelo
        liga('y');
        printk("ATENCAO\n");
        *estado = 1; // PARE
        k_msleep(SLEEP_TIME_MS);
        return;
    }

    return;

}

void main(void)
{
    int ret_g, ret_r, ret_b;

    // Verifica se o device está pronto
    if ((!gpio_is_ready_dt(&led_g)) || (!gpio_is_ready_dt(&led_r)) || (!gpio_is_ready_dt(&led_b)) ){
        printk("Error: some LED device(s) is(are) not ready\n");
        return;
    }

    // Configura o pino como saída
    ret_g = gpio_pin_configure_dt(&led_g, GPIO_OUTPUT_ACTIVE);
    if (ret_g < 0) {
        printk("Error %d: failed to configure LED pin\n", ret_g);
        return;
    }

    ret_r = gpio_pin_configure_dt(&led_r, GPIO_OUTPUT_ACTIVE);
    if (ret_r < 0) {
        printk("Error %d: failed to configure LED pin\n", ret_r);
        return;
    }

    ret_b = gpio_pin_configure_dt(&led_b, GPIO_OUTPUT_ACTIVE);
    if (ret_b < 0) {
        printk("Error %d: failed to configure LED pin\n", ret_b);
        return;
    }

    // Led ativado
    printk("LED blinking on %s pin %d\n", led_g.port->name, led_g.pin);
    printk("LED blinking on %s pin %d\n", led_r.port->name, led_r.pin);
    printk("LED blinking on %s pin %d\n", led_b.port->name, led_b.pin);

    // setando estado inicial da FSM
    int estado = 0;

    while (1) {
        // Chama funcao semaforo
        semaforo(&estado);
    }
}
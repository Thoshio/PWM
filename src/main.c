#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z402.h>                  // Biblioteca personalizada com funções de controle do TPM (Timer/PWM Module)
#include <zephyr/console/console.h>
#include <stdio.h>

// Define o valor do registrador MOD do TPM para configurar o período do PWM
#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))

// Define o LED usando Device Tree
#define green_NODE DT_ALIAS(led0) //verde
#define red_NODE DT_ALIAS(led2) //vermelho

#if DT_NODE_HAS_STATUS(green_NODE, okay) && DT_NODE_HAS_STATUS(red_NODE, okay)
static const struct gpio_dt_spec led_g = GPIO_DT_SPEC_GET(green_NODE, gpios);
static const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(red_NODE, gpios);
#else
#error "Unsupported board: some led devicetree alias is not defined"
#endif

int main(void)
{
    // Inicializa o módulo TPM2 com:
    // - base do TPMx
    // - fonte de clock PLL/FLL (TPM_CLK)
    // - valor do registrador MOD
    // - tipo de clock (TPM_CLK)
    // - prescaler de 1 a 128 (PS)
    // - modo de operação EDGE_PWM
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    // Inicializa o canal 0 do TPM2 para gerar sinal PWM na porta GPIOB_18
    // - modo TPM_PWM_H (nível alto durante o pulso)
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, led_r.pin);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, led_g.pin);

    // Desligando os leds
    pwm_tpm_CnV(TPM2, 0, TPM_MODULE);
    pwm_tpm_CnV(TPM2, 1, TPM_MODULE);

    // Define variaveis para sabermos o value do Duty Cycle
    float value_g, duty_g, value_r, duty_r;

    // Para cor laranja: Duty_green = 50% e Duty_red = 90%
    duty_g = 0.5;
    duty_r = 0.9;

    // Criando a variável de entrada do user que controla 
    // a intensidade do brilho do LED
    int intensity_user;
    float intensity_led;

    // Para o console
    console_getline_init();
    
    // Loop infinito
    for (;;)
    {
        // Lê o valor digitado pelo usuario
        printk("\n\nInsira um valor que determine a intensidade do brilho do LED de 0(min) a 100(max):\n");
        char *line = console_getline();

        if (sscanf(line, "%d", &intensity_user) == 1) {

            // Trava o valor entre 0 e 100 para segurança
            if (intensity_user > 100) intensity_user = 100;
            if (intensity_user < 0) intensity_user = 0;
            printk("Processando brilho: %d \n", intensity_user);

            // Transforma a intensidade do usuário em porcentagem
            intensity_led = intensity_user/100.0;

            // Equacao do value de pwm_tpm_CnV(tpm, channel, value) para Active Low
            value_g = TPM_MODULE * (1 - (duty_g * intensity_led));
            value_r = TPM_MODULE * (1 - (duty_r * intensity_led));

            // Define o valor do duty cycle: nesse caso, duty_100% (LED quase desligado)
            pwm_tpm_CnV(TPM2, 0, (uint16_t)value_r);
            pwm_tpm_CnV(TPM2, 1, (uint16_t)value_g);

        } else {
            // Caso seja enviada uma letra por exemplo
            printk("Entrada invalida. Digite um numero inteiro.\n");
        }
    }

    return 0;
}
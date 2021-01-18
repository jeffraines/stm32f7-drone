## Drone Testing Videos

https://vimeo.com/jeffraines

## STM32F722 Pinout

Motor Outputs
- PA6 (D12) (CN7/Top Right) (TIM3): Motor 1
- PA7 (D11) (CN7/Top Right) (TIM3): Motor 2
- PB0 (D33) (CN10/Bottom Left) (TIM3): Motor 3
- PC9 (D44) (CN8/Top Right) (TIM3): Motor 4
- PD13 (D28) (CN10/Bottom left) (TIM4): Motor 5
- PD14 (D10) (CN7(Top Right) (TIM4): Motor 6

Test Inputs
- PB1 (A6) (CN10/Top Left): ADC input for potentiometer 
- PF14 (D4) (CN10/Top Right): GPIO button input
- PF15 (D2) (CN10/Top Right): GPIO button input

RX Inputs
- PE9 (D6) (CN10/Top Right) (TIM1 CH1): Throttle RX CH3 
- PE11 (D5) (CN10/Top Right) (TIM1 CH2): Pitch RX CH2
- PE13 (D3) (CN10/Top Right) (TIM CH3): Roll RX CH1
- PE14 (D38) (CN10/Bottom Right (TIM1 CH4): Yaw RX CH4
- PA0 (D32) (CN10/Bottom Left) (TIM2 CH1): SwitchA RX CH5
- PA3 (A0) (CN9/Top Left) (TIM2 CH4): SwitchB RX CH6
- PE7 (D41) (CN10/Bottom Right) (TIM1 ETR): External trigger source for input capture compare	

I2C Communication
- PB8 (D15) (CN7/Top Right): SCL
- PB9 (D14) (CN7/Top Right): SDA

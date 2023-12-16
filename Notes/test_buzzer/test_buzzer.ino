// Program testBuzzer
  // mencoba buzzer

int button = 13;
int buzz = 15;
volatile bool gpio_intr_flag = false;


void IRAM_ATTR isr()
{
  gpio_intr_flag = true;
}

void buttonTask(void* pvParameters)
{
  (void)pvParameters;
  while (1)
  {
    if(gpio_intr_flag == true)
    {
      for (int j = 0;j<2;j++)
      {
        digitalWrite(buzz,HIGH);
        vTaskDelay(100);
        digitalWrite(buzz,LOW);
        vTaskDelay(100);
      }
    }
  }
}

void setup()
{
  // put your setup code here, to run once:
  pinMode(button,INPUT);
  pinMode(buzz, OUTPUT);
  attachInterrupt(button, isr, RISING);

  xTaskCreatePinnedToCore(buttonTask, "Button Task", 2048, NULL, 1, NULL, 0);

}
void loop()
{
  
   if (true == gpio_intr_flag)
  {
    gpio_intr_flag = false;    
  }
}

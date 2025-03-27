#include <fpioa.h>
#include <gpio.h>
#include "Maix_Speech_Recognition.h"
#include "voice_model.h"

SpeechRecognizer rec;


#define NOTIFY_PIN   21   
#define THANK_YOU_PIN 22     
#define WATER_PIN 23      

#define NOTIFY_GPIOHS 0    
#define THANK_YOU_GPIOHS 1  
#define WATER_GPIOHS 2   

void setup()
{
    fpioa_set_function(NOTIFY_PIN, FUNC_GPIOHS0);
    fpioa_set_function(THANK_YOU_PIN, FUNC_GPIOHS1);
    fpioa_set_function(WATER_PIN, FUNC_GPIOHS2);

    gpiohs_set_drive_mode(NOTIFY_GPIOHS, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(THANK_YOU_GPIOHS, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(WATER_GPIOHS, GPIO_DM_OUTPUT);

    rec.begin();
    Serial.begin(115200);
    Serial.println("init model...");
    
    rec.addVoiceModel(0, 0, red_0, fram_num_red_0); 
    rec.addVoiceModel(0, 1, red_1, fram_num_red_1); 
    rec.addVoiceModel(0, 2, red_2, fram_num_red_2); 
    rec.addVoiceModel(0, 3, red_3, fram_num_red_3); 
    rec.addVoiceModel(1, 0, green_0, fram_num_green_0);     
    rec.addVoiceModel(1, 1, green_1, fram_num_green_1);     
    rec.addVoiceModel(1, 2, green_2, fram_num_green_2);     
    rec.addVoiceModel(1, 3, green_3, fram_num_green_3);     
    rec.addVoiceModel(2, 0, blue_0, fram_num_blue_0);   
    rec.addVoiceModel(2, 1, blue_1, fram_num_blue_1);   
    rec.addVoiceModel(2, 2, blue_2, fram_num_blue_2);   
    rec.addVoiceModel(2, 3, blue_3, fram_num_blue_3);   
    rec.addVoiceModel(3, 0, turnoff_0, fram_num_turnoff_0);  
    rec.addVoiceModel(3, 1, turnoff_1, fram_num_turnoff_1);  
    rec.addVoiceModel(3, 2, turnoff_2, fram_num_turnoff_2);  
    rec.addVoiceModel(3, 3, turnoff_3, fram_num_turnoff_3);
    rec.addVoiceModel(4, 0, help_0, fram_num_help_0);
    rec.addVoiceModel(4, 1, help_1, fram_num_help_1);
    rec.addVoiceModel(4, 2, help_2, fram_num_help_2);

    rec.addVoiceModel(5, 0, water_0, fram_num_water_0);
    rec.addVoiceModel(5, 1, water_1, fram_num_water_1);
    rec.addVoiceModel(5, 2, water_2, fram_num_water_2);  

    rec.addVoiceModel(6, 0, tq_0, fram_num_tq_0);
    rec.addVoiceModel(6, 1, tq_1, fram_num_tq_1);
    rec.addVoiceModel(6, 2, tq_2, fram_num_tq_2);  
    Serial.println("init model ok!");
}

void loop()
{
    int res = rec.recognize();
    Serial.printf("res : %d ", res);
    if (res > 0){
        switch (res)
        {
        case 5:
            gpiohs_set_pin(NOTIFY_GPIOHS, GPIO_PV_HIGH);  
            gpiohs_set_pin(THANK_YOU_GPIOHS, GPIO_PV_LOW);
            gpiohs_set_pin(WATER_GPIOHS, GPIO_PV_LOW);
            Serial.println("rec : help! ");
            delay(5000);
            gpiohs_set_pin(NOTIFY_GPIOHS, GPIO_PV_LOW);  
            gpiohs_set_pin(THANK_YOU_GPIOHS, GPIO_PV_LOW);
            gpiohs_set_pin(WATER_GPIOHS, GPIO_PV_LOW);
            break;
        case 6:
            gpiohs_set_pin(WATER_GPIOHS, GPIO_PV_HIGH);   
            gpiohs_set_pin(NOTIFY_GPIOHS, GPIO_PV_LOW);   
            gpiohs_set_pin(THANK_YOU_GPIOHS, GPIO_PV_LOW);
            Serial.println("rec : water!");
            delay(5000);
            gpiohs_set_pin(NOTIFY_GPIOHS, GPIO_PV_LOW);  
            gpiohs_set_pin(THANK_YOU_GPIOHS, GPIO_PV_LOW);
            gpiohs_set_pin(WATER_GPIOHS, GPIO_PV_LOW);
            break;
        case 7:
            gpiohs_set_pin(WATER_GPIOHS, GPIO_PV_LOW);   
            gpiohs_set_pin(NOTIFY_GPIOHS, GPIO_PV_LOW);   
            gpiohs_set_pin(THANK_YOU_GPIOHS, GPIO_PV_HIGH); 
            Serial.println("rec : Thank you!");
            delay(5000);
            gpiohs_set_pin(NOTIFY_GPIOHS, GPIO_PV_LOW);  
            gpiohs_set_pin(THANK_YOU_GPIOHS, GPIO_PV_LOW);
            gpiohs_set_pin(WATER_GPIOHS, GPIO_PV_LOW);
            break;
        default:
            gpiohs_set_pin(NOTIFY_GPIOHS, GPIO_PV_LOW);   
            gpiohs_set_pin(THANK_YOU_GPIOHS, GPIO_PV_LOW);
            gpiohs_set_pin(WATER_GPIOHS, GPIO_PV_LOW);
            Serial.println("rec : DEFAULT");
            break;
        }
    } else {
        Serial.println("recognize failed.");
    }
    delay(1000);
}

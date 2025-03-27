
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "SmartMedic"
#define WIFI_PASSWORD "sm@1234567"

#define sensorPin 13
#define FIREBASE_PROJECT_ID "smort-care"
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-1t42h@smort-care.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCfu6VMZxZ+3I+Q\nNcjONIsF7peYevyraXqu3HIPXA0GB4x/AKFkRr3Xie+Bo4/dR2BcfQKAfpLx5P+9\nU6i2cVdhN506bYtGZ7zuPtLQjCnINEZXVi2lWu0uI2OnVFZnrqTdRjobIOsqp5jW\nlG2Wr/Uu3GImu+ZXCLL7WhfPeCW0frhRLtKwNnMgl9lLyyPT9z/34OtkMgf81ovO\nIkZ77hwHvoEafhfwIuTo6j4S89963EUWfk1wDgxkgk4IJ5J3EFgrRDay2MBoFUER\nIYl15D2rSmHvQUh+bcus57/xHp+Av781Dr4PDL6cyv33c8S0FcmCCExPQCScb8Xn\nFfrWuMRBAgMBAAECggEAAa6c5pTtnZL1UKQryHXVeYao+So8rX+2bk+LH/KRftPC\nYW23RWAPnt8D/Dvu2cOLqF/268fE4Bb0jDMjeh4USw6BlhM2q4oiH+neTr8NZrrA\ne3NAcmALLe9YnODqDk7i8CNNnQ6qHZmXE7pCalH2CgVKrQaaYoGtFozPVDyyF88y\nH1ojj6CfLgmxl1CzFuwAjiaJIDfyy5bx5O38cuH0u1F7GW7efoQOTjkf+jdYdq2M\nLlL8OtPd0W2d8weAq0alFLB9/c4TxV9dzSZvRo8GOQLooEuPT7mIhke5tOWQpjjR\nnGrag225DpdEV0wuDKKWfHc1LffD5YTsPRuwxf+jsQKBgQDPRSeDR1c76A6a6FC6\n7vuVrgnZ905nfqz/3a1EvNUR5b2pDzJ5sg+8z2828j1eRy6C2oaHQhMGy/JRRa99\nNo1SSLQ4k+YgDQMpn4Y8z5OVUy4dMRJrT18dI1PoOLL5fAAXv5HOMAjzVbqph1Za\nuRQDkliS+UjeJtD1BsD/sQ3IBQKBgQDFSWa59Jg55AJcKpBbXPlQyGbxPrTUpgCC\n0YBrJDay3Fjdvv/5TCAZh4rZzMLdSqw4qff+GgUrKc8DP/F91SQyD8NoC71QzulG\n5COKYwLXfjccSb7hef3KiGzLCWVnrI6KVQtL+JMqtEOmhhJsEBKvcr2vreYM6V3w\nSd0NmO3sDQKBgAH0Wv0H7TCpbYnUav54RnMQ4xLlHB4puaPoCTw+s5upmPJBLG6t\nWSykoB3ahu2eqjedq+sWmmtT/QL7Lz51BzwhZ2GpM2BV/xOjZ3anVnLwB91Kmyvb\n4b+6l433CukDXFTc/5j8Jvl0c4ApwIy2dhPai29HXoKJxyqoV6WRm9mpAoGBAJ80\nCnfJUAZ3XwgtOOzhbDeqYJgROC/Z/dOTaIQNZy6S7guTlFEDA2xWmtADksSs+6mG\nT594M++O/4Tp1uDqXRkg61lND+rj7g/NDnkgSSzIp4RmZmjh3tHPoat+25v7/dLX\nqhQWdfwiF+2Lh9Y4aju25iect2Z389xyXrYYUPcJAoGAUOIODneu9a/yd5j5zzVW\nIhfdgXtUpSNe+T+cUwIFMKoVOE+tIuxLmpq0Z7LDDGwzqDtXzc98k3OnbWRJAhpP\ntbil4rQmv8hPZRNQ6xKNoui2DQblf5szxu7N2L56sfeK4YF04UlNaZVMTbmCZoMU\nynIUuSFfpz7MIZMaUah45dU=\n-----END PRIVATE KEY-----\n";
#define DEVICE_REGISTRATION_ID_TOKEN "fmXIGOCZQdiBrAlaJQ_DYW:APA91bGG65xzjipe9Yi1sxnc3CEaNWu5ZEl_H0vj8sM1bEX68XxpGojvcE9QivspILxZDdqgSmbyrkztqh2ns9anCGsJyVhIjtMufSqpUfykedbDV70eebdc2uHDY5wRfhsp0k3qQB2n"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long lastTime = 0;

void sendMessage();

void setup()
{
    Serial.begin(115200);
    pinMode(sensorPin, INPUT);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;

    config.token_status_callback = tokenStatusCallback;
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096, 1024);

    Firebase.begin(&config, &auth);
}

void loop()
{
    if (Firebase.ready() && digitalRead(sensorPin) == HIGH)
    {
        sendMessage();
        delay(10000);
    }
}

void sendMessage()
{
    Serial.print("Send Firebase Cloud Messaging... ");

    FCM_HTTPv1_JSON_Message msg;
    msg.token = DEVICE_REGISTRATION_ID_TOKEN;
    msg.notification.body = "Patient asked for help !!!";
    msg.notification.title = "Smort Alert !!!";

    if (Firebase.FCM.send(&fbdo, &msg))
        Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    else
        Serial.println(fbdo.errorReason());
}

#include <SPI.h> // библиотека для общения по интерфейсу SPI
#include "RF24.h" // библиотека для общения wifi модулей (да да не все так просто)

bool radioNumber = 0; // выбираем номер радио 1/0

RF24 radio(7,8); // задаем пины CE CSN

//Плата                         UNO     UNO     MEGA
//Библиотека                    RF24    Mirf    RF24
//GND	1       Коричневый	GND	GND	GND
//VCC	2	Красный	VCC	3.3 V	3.3V	3.3V
//CE	3	Оранжевый	7	9	9
//CSN	4	Желтый		8	10	53    
//SCK	5	Зеленый		13	13	52
//MOSI	6	Синий		11	11	51
//MISO	7	Фиолетовый	12	12      50

byte addresses[][6] = {"1Node","2Node"}; // создаем точки доступа, максимум 6 символов на название

// 1 - отправляем данные 0 - получаем данные
bool role = 0;

void setup() // выполняется 1 раз
{ // начало программы
  Serial.begin(115200); // скорость обмена 115200 бод
  
  radio.begin(); // инициализируем

  // стандартно RF24_PA_MAX, установлено в RF24_PA_LOW для предотвращения проблем с питанием(шумы в питании создают помехи)
  radio.setPALevel(RF24_PA_LOW);
  
  if(radioNumber){ // 
    
    // открывает канал Node2 для записи 
    radio.openWritingPipe(addresses[1]);
    // желательно не использовать 0 канал для чтения (не писать 0 в качестве первого аргумента)
    radio.openReadingPipe(1,addresses[0]);
    // данный код использует Node1 и Node2 для того чтобы задать 2 точки доступа, одну для чтения и одну для записи
    
  }else{
    // данный код инверсный, тоесть в нем все наоборот, для чтения открыт Node1 а для записи Node2
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  // начинаем сеанс чтения
  radio.startListening();
}

void loop() { // выполняется бесконечное количество раз

if (role == 1)  { // отправляем данные
    
    radio.stopListening();  // останавливаем чтение если оно было в предыдущем цикле

    /*
      здесь мы можем подготовить данные
    */
    
    unsigned long start_time = micros(); // получаем текущее время с момента запуска
     if (!radio.write( &start_time, sizeof(unsigned long) )){ // отправляем текущее время в канал
       Serial.println(F("Not sent")); // если не отправлено
     }
        
    radio.startListening();                                    // И продолжаем слушать
    
    unsigned long started_waiting_at = micros();               // снова берем время
    boolean timeout = false;                                   // переменная timeout хранит ошибку, если отправка данных не подтверждена
    
    while ( ! radio.available() ){                             // ожидаем приема ответа от платы, которой отправили данные
      if (micros() - started_waiting_at > 200000 ){            // 200 миллисекунд на ожидание ответа
          timeout = true;
          break;
      }      
    }
        
    if ( timeout ){                                             // в случае когда ответа не дождались
        Serial.println(F("Failed, response timed out."));
    }else{                                                     // если ответ все таки есть
        unsigned long got_time;                                // создаем переменную для хранения полученных данных
        radio.read( &got_time, sizeof(unsigned long) );        // считываем в переменную эти данные
        unsigned long end_time = micros();                     // берем время окончания
        
        /* Здесь мы можем вывести время приема */
    }

    // создаем задержку в 1 секунду
    delay(1000);
  }

  if ( role == 0 ) // если собираемся принимать данные
  {
    unsigned long got_time; // создаем переменную для хранения полученного времени
    
    if( radio.available()){ // если удается открыть канал связи
    
      while (radio.available()) {                                   // пока вы получаете данные
        radio.read( &got_time, sizeof(unsigned long) );             // считываем их (советую читать данные пакетами)
      }
     
      radio.stopListening();                                        // останавливаем прослушивание
      radio.write( &got_time, sizeof(unsigned long) );              // отправляем обратно полученное время      
      radio.startListening();                                       // начинаем прослушивать канал заново
   }
 }

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ){      
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;
    
   }else
    if ( c == 'R' && role == 1 ){
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));      
       role = 0;
       radio.startListening();
       
    }
  }


}


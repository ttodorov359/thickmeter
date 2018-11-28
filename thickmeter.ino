// программа - Измеритель-толщины покрытия. 
// идея отсюда: https://github.com/sae/Arduino-LCQmeter/blob/master/LC-gen.ino
/*
simplest LC-generator on arduino
can be used as lc-meter
(C)SAE762 07.feb.2015 @home
circuit diagramm D2---R(100...1000)---D6---LC---GND; D7---GND
*/

#include <EEPROM.h> // библиотека чтения-записи eeprom 
#include "analogComp.h" // библиотека, скчана с гитхаба

/* проверка работы eeprom
 */
 //***************** ФУНКЦИИ***********
 //This function will write a 2 byte integer to the eeprom at the specified address and address + 1
 // эта функия записывает двухбайтовое целое число в РППЗУ по указанному адресу

void EEPROMWriteInt(int p_address, int p_value)
 
        {
        byte lowByte = ((p_value >> 0) & 0xFF);
        byte highByte = ((p_value >> 8) & 0xFF);

        EEPROM.write(p_address, lowByte);
        EEPROM.write(p_address + 1, highByte);
        }

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
// эта функия считывает двухбайтовое целое число в РППЗУ по указанному адресу

unsigned int EEPROMReadInt(int p_address)
        {
        byte lowByte = EEPROM.read(p_address);
        byte highByte = EEPROM.read(p_address + 1);

        return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
        }
//********конец объявления фунций************
//обявление переменных     
int ctl=2 ; // нога для накачки энергии в контур
int led=13; // нога светодиода

int rise=0; // счетчик переходов через 0
int fall=0; // такой же стетчик
int summ; //накопительный счетчик
int tchk=300; // время измерения
int zero=11625; //отладка
int delta=0; //разница между содержимым eeprom и результатом измерения

//присвоим номера пинов светодиодов
int vbat=0;  //напряжение батареи
int vbatpin=6; // аналог вход 6
int mredled=3; // красный светодиод -
int mblueled=4; //синий светодиод -
int mwhiteled=5;//белый светодиод -
int greenled=8;//зеленый светодиод
int pwhiteled=9;//белый светодиод +
int pblueled=10; //синий +
int predled=11; //красный +
// присвоим границы 
//++++++++++++++++++++++++++++++++
// тут придется внести ВАШИ значения!!!!
//===============================
int pgreen=25; // граница ОК  в плюс
int mgreen=-10;// в минус
int pwhite=55; // граница подозрительно в +
int mwhite=-30;// подозрительно в -
int pblue=100; // крашено 
int mblue=-60; // шлифовано
//int pred=130; // шпаклевка
//int mred=-60;//металл



void setup() {
  // put your setup code here, to run once:
  summ=0;// инициализация накопителя
   Serial.begin(9600); // настроим порт УБРАТЬ ПОСЛЕ ОТЛАДКИ
   pinMode(led, OUTPUT); // 
   pinMode(ctl, OUTPUT); // инициализация выхода накачки
      //настроим выхода под светодиоды
   pinMode(mredled, OUTPUT);
   digitalWrite(mredled, HIGH);
   pinMode(mblueled, OUTPUT);
   digitalWrite(mblueled, HIGH);
   pinMode(mwhiteled, OUTPUT); 
   digitalWrite(mwhiteled, HIGH);
   pinMode(greenled, OUTPUT); 
   digitalWrite(greenled, HIGH);
   pinMode(pwhiteled, OUTPUT); 
   digitalWrite(pwhiteled, HIGH);
   pinMode(pblueled, OUTPUT);
  digitalWrite(pblueled, HIGH); 
   pinMode(predled, OUTPUT); 
  digitalWrite(predled, HIGH); 
   
// тут можно дорожку сделать для индикации светодиодов, если очень хочется
// инициализация компаратора и прерываний
   digitalWrite(ctl, LOW);
  analogComparator.setOn(AIN0, AIN1);//D6+,D7- 
  analogComparator.enableInterrupt(comparatorInt,CHANGE);
  // проверка режима работа/установка 0
   vbat = analogRead(vbatpin);     // считываем значение 
  if (vbat>50) {  // если напруга больше 0 запускаем установку 0
  rise=0;
  // индикация разряженной батареи, но установка 0 продолжается
  if (vbat < 700) {digitalWrite(mredled, LOW);digitalWrite(predled, LOW);}
  //if no pulses, make a start impulse
  if (rise==0) {
    pulse();
  }
    // первый проход
  summ=0;
digitalWrite(led, LOW);   
  rise=0;
  fall=0;
  delay(tchk);              // wait for a second
  //Serial.println(rise);    // вывод на псолед порт, убрать после отладки
  summ=summ+rise;
  // второй проход
  rise=0;
  fall=0;
  delay(tchk);              // wait for a second
  summ=summ+rise;
  //третий проход
   rise=0;
  fall=0;
  delay(tchk);              // wait for a second
  summ=summ+rise-15;  // -15 странный костыль видимо недосчет во время обработки прерывания
//  Serial.print(summ);  // убрать после отладки
  //вычислим среднее
  summ=summ/3;  // вот это и есть измренное значение по результатам трех проходов
// это условная толщина покрытия в попугаях
// зная ее, можно с помомощью нехитрых арифметических действий, определить реальную толщину покрытия
//то есть померить калиброванную пленку разной толщины, записать результаты, а потом подобрать коэффициенты.

  digitalWrite(led, HIGH); 
    EEPROMWriteInt(0,summ); //занесем в eeprom 
    digitalWrite(greenled, LOW); // зажжем зелененький
    while (1); // застопорить нахрен после установки 0 !!!!!!!
                  }
}
// конец процедуры установки  
 // тут переход к процедуре измерения 
//!!!!!Главный цикл!!!
void loop() {
  // тут процедура измерения
  rise=0;
  //Serial.println(EEPROMReadInt(0));             // выводим полученное значение
  //if no pulses, make a start impulse
  if (rise==0) {
    pulse();
  }
  // первый проход
  summ=0;
digitalWrite(led, LOW);   
  rise=0;
  fall=0;
  delay(tchk);              // wait for a second
  summ=summ+rise;
  // второй проход
 rise=0;
  fall=0;
  delay(tchk);              // wait for a second
  summ=summ+rise;
  //третий проход
   rise=0;
  fall=0;
  delay(tchk);              // wait for a second
  summ=summ+rise;
//  Serial.print(rise);
  //вычислим среднее
  summ=summ/3;
  delta=summ-EEPROMReadInt(0);
Serial.print(EEPROMReadInt(0));             // из памяти  
Serial.print(";");
 Serial.print(delta);
  Serial.println(" pulses/tchk");
  digitalWrite(led, HIGH);    
 if (delta >= mgreen && delta <= pgreen ) {digitalWrite(greenled, LOW);} 
 if (delta < mblue) digitalWrite(mredled, LOW);
 if (delta >= mblue && delta < mwhite ) digitalWrite(mblueled, LOW); 
 if (delta >= mwhite && delta < mgreen) digitalWrite(mwhiteled, LOW);
 if (delta > pblue) digitalWrite (predled,LOW);
 if (delta > pwhite && delta <= pblue) digitalWrite (pblueled,LOW);
 if (delta > pgreen && delta <= pwhite) digitalWrite (pwhiteled,LOW); 
 while (1);  /// тормоза. измерение окончено
 delay(1000);              // wait for a second


}





/**
not so clear "magic" with interrupt
we need to block int's when make a power impulse
comparator is connected directly to capacitor
*/
boolean waitforRise=false;

//this function is called when comparator change status
//as set in ACSR register
void comparatorInt() {
  
  if (waitforRise) { //if we waiting for rise
    ACSR &= ~(1<<ACIS0);//set interrupt for falling
    ACSR |= (1<<ACIS1);
    waitforRise=false;
    rise++;
  } else { //if we waiting for fall
   ACSR &= ~(1<<ACIE); //disable interrupt
    //impulse to support an oscillations
   pulse();
   ACSR |= (1<<ACIE); //enable interrupt
   ACSR |= ((1<<ACIS1) | (1<<ACIS0)); //set interrupt for rising
   waitforRise=true;
  }
}

//send power impulse to LC
void pulse() {
   pinMode(ctl, OUTPUT);
   digitalWrite(ctl, HIGH);
   rise++;//make something useless )
   //digitalWrite(ctl, LOW);
   pinMode(ctl, INPUT);
}

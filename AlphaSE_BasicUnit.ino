/*
  Базовая версия программы для модуля расширения системы AlphaSE
  Модуль не имеет никакого функционала, но уже может опрашиваться контроллером по протоколам
  ADNet и ModBus RTU.
  Данную программу можно использовать в качестве основы для разработки новых модулей.

  Описание PIN
  0 - RX RS485 RO
  1 - TX RS485 DI
  2 - управление приёмом и передачей по RS485. На этой же ноге висит Led индикации передачи
  3 - Pin 1 для задания адреса модуля по шине ADNet/ModBus
  4 - Pin 2 для задания адреса модуля по шине ADNet/ModBus
  5 - Pin 3 для задания адреса модуля по шине ADNet/ModBus
  6 - Pin 4 для задания адреса модуля по шине ADNet/ModBus
  7 - Pin 5 для задания адреса модуля по шине ADNet/ModBus
  8 - Pin 6 для задания адреса модуля по шине ADNet/ModBus
  Адрес модуля вычисляется по формуле Pin1+Pin2*2+Pin3*4+Pin4*8+Pin5*16+Pin6*32
  13 - Led индикации работы. Мигает 1 раз в секунду, если всё хорошо, иначе - ошибка

  Подробнее о системе Alpha SE можно узнать на сайте smart-elec.ru
  Автор: Быков Виктор Сергеевич
  Дата 02.10.2018г.
*/
#include <alphase_unit.h>

  alphase_unit unit; // переменная класса типа модуль
  unsigned char point0_pin;  // Вход №0
  unsigned char point1_pin;  // Вход №1
  unsigned char point2_pin;  // Вход №2
  unsigned char point3_pin;  // Вход №3
  unsigned char point4_pin;  // Вход №4
  unsigned char point5_pin;  // Вход №5

  unsigned char point8_pin;  // Выход №0
  unsigned char point9_pin;  // Выход №1
  unsigned char point10_pin;  // Выход №2
  unsigned char point11_pin;  // Выход №3
  unsigned char point12_pin;  // Выход №4

void setup() {
  // put your setup code here, to run once:
  unit.start();

  unit.unit_ver=1; //Версия модуля
  unit.unit_type=1; // Тип модуля
  unit.unit_subtype=0; // Подтип модуля
  /*
   Type/Subtype - Name
    1/0 - SE Light
    2/0 - SE Humb
    3/0 – SE Temp
    4/0 – SE Pressure
    5/0 – SE Humd&Temp
    11/33 – Secu 16
    12/0 – Secu 16I
    13/0 – reserved
    14/0 - reserved
    16/0 - SE 2ai 0-5v
    17/0 - SE 11i
    18/0 - SE Secu
    19/0 - SE 2o 0-10V
    20/0 - SE 1i 0-10V
    21/0 - SE Curtain RO
    22/0 - SE Curtain PO
    30/0 - SE 6i5o
    31/0 - SE RoomBlock   
   */

  point0_pin = A7;  // Вход №0
  point1_pin = A6;  // Вход №1
  point2_pin = A5;  // Вход №2
  point3_pin = A4;  // Вход №3
  point4_pin = A3;  // Вход №4
  point5_pin = A2;  // Вход №5

  point8_pin = 9;  // Выход №0
  point9_pin = 10;  // Выход №1
  point10_pin = 11;  // Выход №2
  point11_pin = 12;  // Выход №3
  point12_pin = A0;  // Выход №4

  pinMode(point0_pin,INPUT);
  pinMode(point1_pin,INPUT);
  pinMode(point2_pin,INPUT);
  pinMode(point3_pin,INPUT);
  pinMode(point4_pin,INPUT);
  pinMode(point5_pin,INPUT);
  pinMode(point8_pin,OUTPUT);
  pinMode(point9_pin,OUTPUT);
  pinMode(point10_pin,OUTPUT);
  pinMode(point11_pin,OUTPUT);
  pinMode(point12_pin,OUTPUT);

  digitalWrite(point0_pin,HIGH); // Включили подтягивающий резистор 20кОм на + 
  digitalWrite(point1_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
  digitalWrite(point2_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
  digitalWrite(point3_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
  digitalWrite(point4_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
  digitalWrite(point5_pin,HIGH); // Включили подтягивающий резистор 20кОм на +

  digitalWrite(point8_pin,LOW); // Выключили реле
  digitalWrite(point9_pin,LOW); // Выключили реле
  digitalWrite(point10_pin,LOW); // Выключили реле
  digitalWrite(point11_pin,LOW); // Выключили реле
  digitalWrite(point12_pin,LOW); // Выключили реле  
  
  unit.serial_active=1; // Включаем обработку данных, приходящих через RS485
}
//Функция возрващеает значение параметра по его номеру
//Часть параметров хранится в EEPROM, другие в обычынх переменных
unsigned char GetUnitParam(unsigned char param_num) // Чтение параметра модуля
{
  if (param_num>255) return 0;
  switch (param_num)
  {
    case 0: return unit.unit_ver;
    case 1: return unit.addr;
    case 2: return unit.unit_type;
    case 4: return unit.unit_subtype;
    default:
    {}
  }
  return unit.EEPROMread(param_num);
}

void SetUnitParam(unsigned char param_num, unsigned char value) // Запись параметра модуля
{
  if (param_num>255) return;
  unit.EEPROMwrite(param_num,value);
};

void loop() {
  // put your main code here, to run repeatedly:
  unit.loop();

  // Обработка команд ADNet
  if (unit.last_cmd_protocol==1) //Если есть команда, поступившая по протоколу ADNet
  {
    switch (unit.cmd_func_id) // Код команды, которую требуется выполнить
    {
      case 5: // Запрос значения параметра модуля
        //Входящие данные      
        //unit.cmd_param[0] Номер параметра, который запросил контроллер
        //Исходящие данные. В массив output_data нужно присвоить значение для отправки контроллеру
        //unit.output_data[0] значение параметра;
        //unit.output_data[1] номер параметра
        //unit.output_data[2] должен быть равен 0
        unit.output_data[0]=GetUnitParam(unit.cmd_param[0]);
        unit.output_data[1]=unit.cmd_param[0];
        unit.output_data[2]=0;
        unit.SendAnswer_ADNet();
        break;
      case 6: // изменить значение параметра        
        //Входящие данные
        //unit.cmd_param[0] Номер параметра в который нужно присвоить значение
        //unit.cmd_param[1] Значение, которое нужно присвоить
        //Исходящие данные для передачи контроллеру не требуются
        //unit.output_data[0] значение параметра;
        //unit.output_data[1] номер параметра
        //unit.output_data[2] должен быть равен 0        
        SetUnitParam(unit.cmd_param[0], unit.cmd_param[1]); // Запись параметра модуля
        unit.output_data[0]=GetUnitParam(unit.cmd_param[0]); // Значение изменяемого параметра
        unit.output_data[1]=unit.cmd_param[0]; // Номер изменяемого параметра
        unit.output_data[2]=0;        
        unit.SendAnswer_ADNet();
        break;
      case 10: // изменение выходов        
        //unit.cmd_param[0] Тип действия. 0 - включение по маске, 1 - выключение по маске, 2 - присвоение маск
        //unit.cmd_param[1] Значение Маски      
        //Исходящие данные. В массив output_data нужно присвоить значение для отправки контроллеру
        //unit.output_data[0] состояние выходов после выполения команды;
        //unit.output_data[1] должен быть равен 0
        //unit.output_data[2] должен быть равен 0
        //!!!
        // Здесь вписать программу управления выходами
        //!!!
        // В ответ нужно отправить итоговое состояне выходов
        //SetData(1, unsigned char value);
        unit.output_data[0]=unit.GetData(1); // Состояние выходов после выполнения команды
        unit.output_data[1]=0;
        unit.output_data[2]=0;
        unit.SendAnswer_ADNet();
        break;
      case 11: // Запрос состояния входов/выходов
        //Исходящие данные. В массив output_data нужно присвоить значение для отправки контроллеру
        //unit.output_data[0] состояние входов
        //unit.output_data[1] должен быть равен 0        
        //unit.output_data[2] состояние выходов
        unit.output_data[0]=unit.GetData(0); //Состояние входов (data[0])
        unit.output_data[1]=0;
        unit.output_data[2]=unit.GetData(1); //Состояние выходов (data[1])
        unit.SendAnswer_ADNet();
        break;
        
       default:{}
    }
  }

  // Обработка команд ModBus RTU
  // Переменные для протокола ModBus RTU
  unsigned short reg_num;
  unsigned short reg_count;
  unsigned short reg_value;
  unsigned char uch_i; // Временная переменная

  if (unit.last_cmd_protocol==2) //Если есть команда, поступившая по протоколу ModBus RTU
  {
    reg_num=unit.cmd_param[0]*256+unit.cmd_param[1]; // Начальный номер параметра, который запросил контроллер
    reg_count=unit.cmd_param[2]*256+unit.cmd_param[3]; // Количество параметров, которые запросил контроллер

    switch (unit.cmd_func_id) // Код команды, которую требуется выполнить
    {
      case 3: // Чтение регистра
        //Входящие данные      
        //Исходящие данные. В массив output_data нужно присвоить значение для отправки контроллеру
        //unit.output_data[0-15] значение параметра;
        for (uch_i=0;uch_i<reg_count;uch_i++)
          unit.output_data[uch_i]=GetUnitParam(reg_num+uch_i);
        unit.SendAnswer_ModBus();
        break;
      case 16: // Запись регистра
        //Входящие данные
        //unit.cmd_param[0] Номер параметра в который нужно присвоить значение
        //unit.cmd_param[1] Значение которое нужно присвоить
        reg_value=unit.cmd_param[4]*256+unit.cmd_param[5]; // значение регистра
        SetUnitParam(reg_num, reg_value); // Запись параметра модуля
        //Исходящие данные для передачи контроллеру не требуются
        unit.SendAnswer_ModBus();
        break;
       default:{}
    }
  }

}
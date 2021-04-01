/*
	Данный модуль предназначен для написания прошивок оборудования системы автоматизации AlphaSE.
	Прошивка поддерживает одновременную работу по протоколам ADNet+ и ModBus RTU.
	Дата последних изменений библиотеки 22.04.2019г.
*/
#include "alphase_unit.h"

alphase_unit::alphase_unit()
{
	serial_active=0; // Обработка посупивших по RS485 данных вестись не будет
	RS485out_pin = 2; // Порт управления прием/передача RS485
	ledWork_pin = 13;  // Светодиод индикации
	jmp_pin[0] = 3;  // Джампер 0
	jmp_pin[1] = 4;  // Джампер 1
	jmp_pin[2] = 5;  // Джампер 2
	jmp_pin[3] = 6;  // Джампер 3
	jmp_pin[4] = 7;  // Джампер 4
	jmp_pin[5] = 8;  // Джампер 5
	jmp_pin[6] = 0;  // Джампер 6 данный pin джампера не является обязательным и используется, если !==0
	jmp_pin[7] = 0;  // Джампер 7 данный pin джампера не является обязательным и используется, если !==0

	for (ush_i=0; ush_i<unit_params_count; ush_i++)
	{
		ee_params_init[ush_i]=0;
		read_only_params[ush_i]=0;
	}
	
	data[0]=0; // Начение поля data0
	data[1]=0; // Начение поля data1
	
	read_only_params[0]=1; // Версия модуля
	read_only_params[1]=1; // Адрес модуля
	read_only_params[2]=1; // Тип модуля
	read_only_params[3]=1; // Подтип модуля
	
	unit_type=1; //Тип модуля. Не должен равняться 0. Иначе модуль не будет опрашиваться контроллером.
	unit_subtype=0; //Подтип модуля (при наличии модификаций одного модуля в разных исполнениях)
	unit_ver=1; //Версия модуля. Не должен равняться 0. Иначе модуль не будет адресоваться контроллером.	
	on_timer_1s=false; // флаг таймера снят
	last_cmd_protocol=0; // Команд ни по одному протоколу не поступало
	answer_delay=0; // Задержка перед ответом = 0
};

// Инициализаиует переменные и запускает начало обмена
void alphase_unit::start()
{
  ledWork_freq=1000; // Частот переключения LED работы 1000 - 1 переключение в секунду  
  pinMode(RS485out_pin,OUTPUT);  
  pinMode(ledWork_pin,OUTPUT);
	for (int i=0;i<8;i++)
		if (jmp_pin[i]!=0)
		{
			pinMode(jmp_pin[i],INPUT);
			digitalWrite(jmp_pin[i],HIGH);
		}
		/*
  pinMode(jmp0_pin,INPUT);
  pinMode(jmp1_pin,INPUT);
  pinMode(jmp2_pin,INPUT);
  pinMode(jmp3_pin,INPUT);
  pinMode(jmp4_pin,INPUT);
  pinMode(jmp5_pin,INPUT);
	if (jmp6_pin!=0) // Джампер 6 не является обязательным и используется, если !==0
		pinMode(jmp6_pin,INPUT);
	if (jmp7_pin!=0) // Джампер 7 не является обязательным и используется, если !==0
		pinMode(jmp7_pin,INPUT);
	*/
	digitalWrite(RS485out_pin,LOW);  // Переходим в прием сообщений
	/*
  digitalWrite(jmp0_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
  digitalWrite(jmp1_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
  digitalWrite(jmp2_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
  digitalWrite(jmp3_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
  digitalWrite(jmp4_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
  digitalWrite(jmp5_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
	if (jmp6_pin!=0) // Джампер 6 не является обязательным и используется, если !==0
		digitalWrite(jmp6_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
	if (jmp7_pin!=0) // Джампер 7 не является обязательным и используется, если !==0
		digitalWrite(jmp7_pin,HIGH); // Включили подтягивающий резистор 20кОм на +
	*/	
  // initialize serial:
  Serial.begin(9600);
  delay(3000);  
  
  if (EEPROMread(0)!=ee_params_init[0]) // Если EEPROM не задан
  {
    //присваиваем начальные значения
    for (ush_i=0; ush_i<unit_params_count; ush_i++)
    {
			EEPROMwrite(ush_i,ee_params_init[ush_i]);
    };
  };
  EEPROMfix(); // Проверяем правильность значений в eeprom

  // Инициализация CRC ModBus
  init_crc16_tab( );
  clear_buff(); // Чистим буфер данных
  
};

void alphase_unit::loop() //Функция, которая должна выполняться в loop() основной программы
{
  addr=GetAddr(); // Вычисляем адрес модуля по перемычкам
	
  if (millis()-last_ledWork>ledWork_freq) // Мигание светодиода работы
  {
    last_ledWork=millis();
    digitalWrite(ledWork_pin,!digitalRead(ledWork_pin)); // Включаем светодиод
  };
  
  if (abs(millis()-timer_1s)>1000) // Если прошла секунда с прошлого срабатывания таймера
  {
    timer_1s=millis();
    on_timer_1s=true;
  }
	else
	{
		on_timer_1s=false;
	};
	
	serialEvent(); //проверяем наличие новых данных в интерфейсе шины
}

// ------------------Функция чистки буфера-----------------------
void alphase_unit::clear_buff() // Функция чистит буфер
{
	// Весь буфер чистить не обязательно
	// Входящие команды находятся только в первых 11 байтах
  for(uch_i=0;uch_i<11;uch_i++) 
    buff[uch_i]=0;
};

// ------------------Функция передачи сообщений по RS-485-----------------------
void alphase_unit::transmit(unsigned char len)
{
  digitalWrite(RS485out_pin,HIGH); // Переключаемся в передачу
	delay(answer_delay); // Задержка перед ответом
  delay(2); // Если это убрать, периодически контроллер неправильно принимает данные
  Serial.write(buff, len); // Передаем ответ
  Serial.flush(); // Ожидаем отправку всей посылки
  //delay(5);
  digitalWrite(RS485out_pin,LOW); // Переключаемся в приём  
	clear_buff(); // Чистим буфер	
}

// ------------------Приём данных-----------------------
//#define ADNet_msg_len 8  // Длина одной команды ADNet (8 символов)
//#define ModBus_read_msg_len_in 8  // Длина одной команды Read Holding Registers (чтение - 8 символов)
//#define ModBus_preset_msg_len_in 11  // Длина одной команды Preset Multiple Regs (запись - 11 символов) 

void alphase_unit::serialEvent() {
	// Если обработка не должна вестись
	if (serial_active==0)
	{
		buff[10]=(char)Serial.read();
		return;
	}
	last_cmd_protocol=0; // Пока неизвестен протокол получаемой команды
  while (Serial.available()) 
  {
    memcpy(&buff[0],&buff[1],10); // Смещаем все на 1 имвол вперед
    buff[10]=(char)Serial.read(); // Добавили символ в буфер

    // Проверяем на наличие сообщений ADNet
    if ((buff[3]==255)&&(buff[4]==255)) // Если есть 2 стартовых байта 255 и 255
    {
      temp_uch=calc_ad_crc(&buff[3]); // Подсчет CRC в протоколе ADNet
      if (buff[10]==temp_uch) //Если crc совпадает
      {
        if (buff[6]==addr) // Если адрес модуля совпадает
        {
					last_cmd_protocol=1; // Команда пришла на протоколе ADNet
          memcpy(&buff[0],&buff[3],8); // Переносим команду в начало буфера
          func_ADNet(); // Заполняем массив данных о пришедшей команде
          break; // Выходим из цикла
        };
      };
    };
    
    // Проверяем на наличие сообщений ModBus Read
    if ((buff[3]==addr) && (buff[4]==3)) // Если первый символ - адрес, второй - команда чтения
    {
      ush_i=calc_mb_crc(&buff[3],6); //Вычисляем контрольную сумму
      memcpy(&buff[0],&ush_i,2);
      if (memcmp(&buff[9],&ush_i,2)==0) // Если CRC совпала
      {
				last_cmd_protocol=2; // Команда пришла на протоколе ModBus
        memcpy(&buff[0],&buff[3],8); // Переносим команду в начало буфера
        func_ModBus(); // выполняем команду, которую прислали на выполение
        break; // Выходим из цикла        
      };
    }

    // Проверяем на наличие сообщений ModBus Preset
    if ((buff[0]==addr) && (buff[1]==16)) // Если первый символ - адрес, второй - команда записи
    {
      ush_i=calc_mb_crc(&buff[0],9); //Вычисляем контрольную сумму
      if (memcmp(&buff[9],&ush_i,2)==0) // Если CRC совпала
      {
				last_cmd_protocol=2;
        func_ModBus(); // выполняем команду, которую прислали на выполение
        break; // Выходим из цикла        
      };
    }    
  };
};
// ---------------------Обработчик команд ADNet--------------------------------
// Функция обработчика сообщений RS-485, полученных по протоколу ADNet
void alphase_unit::func_ADNet()
{
  unsigned char uch_i0,uch_i1,i;
  buff[3]=0;
	cmd_func_id=buff[2]; // код команды
  switch (buff[2])
  {
  case 0: // Запрос типа и версия модуля
		last_cmd_protocol=0; // Команда обрабатывается автоматически внутри библиотеки. О событии в основную программы не сообщается.
    buff[4]=unit_ver;
    buff[5]=unit_type;
		buff[6]=unit_subtype;
    buff[7]=calc_ad_crc(&buff[0]);
    transmit(8);
    break;
  case 5: // вернуть значение параметра
		cmd_param[0]=buff[5]; // Номер параметра, который нужно вернуть
    break;
  case 6: // изменить значение параметра
    if((buff[5]>=unit_params_count) || // Если параметр больше максимального
		(read_only_params[buff[5]]==1)) // или он read_only
		{
			last_cmd_protocol=0; //true, если поступила команда AdNet      
			buff[7]=calc_ad_crc(&buff[0]);
			transmit(8);
		}
		else
		{
			cmd_param[0]=buff[5]; // Номер параметра в который нужно присвоить значение
			cmd_param[1]=buff[4]; // Значение которое нужно присвоить
		}
    break;
  case 10: // изменение выходов
  case 51:
		cmd_param[0]=buff[5]; // Тип действия. 0 - включение по маске, 1 - выключение по маске, 2 - присвоение маски
		cmd_param[1]=buff[4]; // Значение Маски
    break;
  case 11: // запрос состояния входов/выходов
  case 50:
		//Команда запроса состояния входов и выходов.
		//Обататывается в теле основной программыё
//    buff[4]=data[0];
//		buff[6]=data[1];
//    buff[7]=calc_ad_crc(&buff[0]);
//    transmit(8);
    break;
  default:
  break;
  }
}
// Функция отправки сообщений ADNet контроллеру
void alphase_unit::SendAnswer_ADNet()
{
  buff[4]=output_data[0];
  buff[5]=output_data[1];		
  buff[6]=output_data[2];	
  buff[7]=calc_ad_crc(&buff[0]);
	transmit(8);
}
// ------------------ Обработчик команд ModBus --------------------------
// Функция обработка сообщений RS-485
void alphase_unit::func_ModBus()
{
  unsigned short reg_num;
  unsigned short reg_count;
  reg_num=buff[2]*256+buff[3];  
  reg_count=buff[4]*256+buff[5];  
	
	cmd_func_id=buff[1]; // код команды	
	cmd_param[0]=buff[2]; 
	cmd_param[1]=buff[3]; 
	cmd_param[2]=buff[4]; 
	cmd_param[3]=buff[5];
	cmd_param[4]=0; 
	cmd_param[5]=0; 	
	
  if (cmd_func_id==3) // Если функция чтения
  {
    if (reg_count>16) last_cmd_protocol=0; // Не можем отправлять более 16 регистров
	}
  if (cmd_func_id==16) // Если функция записи
  {
    if (reg_count>1) last_cmd_protocol=0; // Не можем редактировать более 1 регистра
    if (buff[6]>2) last_cmd_protocol=0; // Количество байт данных не может быть больше 2 
		cmd_param[4]=buff[7]; 
		cmd_param[5]=buff[8]; 	
	}
	return;
}

// Функция отправки сообщений ModBus контроллеру
void alphase_unit::SendAnswer_ModBus()
{
  unsigned char i;
  unsigned short crc;
  unsigned short reg_num;
  unsigned short reg_count;
  reg_num=cmd_param[0]*256+cmd_param[1];  
  reg_count=cmd_param[2]*256+cmd_param[3];  
  
  if (cmd_func_id==3) // Если функция чтения
  {
		// Данныая проверка есть при получении сообщения
    //if (reg_count>16) return; // Не можем отправлять более 16 регистров
    buff[2]=reg_count*2; // Количество отправляеммых байт данных в ответе
    for (i=0;i<reg_count;i++)
    {
      buff[3+i*2]=0; // Всегда будет 0 т.к. тип параметра unsigned char ee_params_c[reg_num+i] /256;      
      buff[4+i*2]=output_data[i];
    }
    crc=calc_mb_crc(&buff[0],3+reg_count*2);
    memcpy(&buff[3+reg_count*2],&crc,2);
    transmit(5+reg_count*2);
  }

  if (cmd_func_id==16) // Если функция записи
  {
		// Данные проверки есть при получении сообщения
    //if (reg_count>1) return; // Не можем редактировать более 1 регистра
    //if (buff[6]>2) return; // Количество байт данных не может быть больше 2 
    crc=calc_mb_crc(&buff[0],6);
    memcpy(&buff[6],&crc,2);
    transmit(8);    
  }
  return;
}

// ------------------Вычисление адреса модуля--------------------------
unsigned char alphase_unit::GetAddr()
{
  unsigned char addr=0;

  if (digitalRead(jmp_pin[0])==LOW)
    addr+=1;
  if (digitalRead(jmp_pin[1])==LOW)
    addr+=2;
  if (digitalRead(jmp_pin[2])==LOW)
    addr+=4;
  if (digitalRead(jmp_pin[3])==LOW)
    addr+=8;
  if (digitalRead(jmp_pin[4])==LOW)
    addr+=16;
  if (digitalRead(jmp_pin[5])==LOW)
    addr+=32;    
	if (jmp_pin[6]!=0) // Джампер 6 не является обязательным и используется, если !==0
		if (digitalRead(jmp_pin[6])==LOW)
			addr+=64;    
	if (jmp_pin[6]!=0) // Джампер 7 не является обязательным и используется, если !==0
		if (digitalRead(jmp_pin[7])==LOW)
			addr+=128;    
		
  if (addr>247) return 255; // Адреса 248…255 — зарезервированы в протоколе ModBus;
  if (addr==0) return 255;
  return addr;      
}
// ----------------- ModBus CRC

// pay attention
// P_16 -- 0xA001 or 1010 0000 0000 0001 is a reversed form
// of the standard polynom 0x8005 or 1000 0000 0000 0101
#define  P_16        0xA001
static unsigned short   crc_tab16[ 256 ];

/*******************************************************************\
*                                                                   *
*   static void init_crc16_tab( void );                             *
*                                                                   *
*   The function init_crc16_tab() is used  to  fill  the  array     *
*   for calculation of the CRC-16 with values.                      *
*                                                                   *
\*******************************************************************/
 
void alphase_unit::init_crc16_tab()
{
  unsigned int i, j;
  unsigned short crc, c;
  for ( i=0; i < 256; ++i )
  {
    crc = 0;
    c = i;
    for ( j = 0; j < 8; ++j )
    {
      if ( ( crc ^ c ) & 0x0001 )
      {
        crc = ( crc >> 1 ) ^ P_16;
      }
      else
      {
        crc =   crc >> 1;
      }
      c = c >> 1;
    }
  crc_tab16[ i ] = crc;
}
   
}  /* init_crc16_tab */
 
 
/*******************************************************************\
*                                                                   *
*   unsigned short update_crc_16( unsigned short crc, char c );     *
*                                                                   *
*   The function update_crc_16 calculates a  new  CRC-16  value     *
*   based  on  the  previous value of the CRC and the next byte     *
*   of the data to be checked.                                      *
*                                                                   *
\*******************************************************************/
 
unsigned short alphase_unit::update_crc_16( unsigned short crc, char c )
{
    unsigned short tmp, short_c;
    short_c = 0x00ff & ( unsigned short ) c;
    tmp =  crc         ^ short_c;
    crc = ( crc >> 8 ) ^ crc_tab16[ tmp & 0xff ];
    return crc;
}  /* update_crc_16 */

//-----------

// Вычисление контрольной суммы ModBus
// Параметром передается ссылка на начало сообщения
unsigned short alphase_unit::calc_mb_crc(unsigned char *buf, unsigned char count)
{
  int i;
  unsigned short crc_16_modbus;
  
  crc_16_modbus  = 0xffff; // pay attention to initialization  
  for ( i = 0; i < count; ++i )
  {
    crc_16_modbus = update_crc_16( crc_16_modbus, buf[ i ] );
  }

  return crc_16_modbus;
}
// ------------------ Вычисление контрольной суммы для модуля-----------
unsigned char  alphase_unit::calc_ad_crc(unsigned char *buff_in) // Вычисление crc для ADNet
{
  unsigned char i, crc[3];
  crc[1]=0;
  for(i=2;i<7;i++)
    crc[1]=crc[1]+buff_in[i];
  return crc[1];
}
// ------------------Чтение EEPROM-----------------------
unsigned char alphase_unit::EEPROMread(unsigned char num)
{
  // В EEPROM могут храниться данные только unsigned char
  unsigned char res_0, res_1, res_2, total_res;
  //if (num>=unit_params_count) return 255;
  res_0=EEPROM.read(num);
  res_1=EEPROM.read(unit_params_count+num);  
  res_2=EEPROM.read(unit_params_count*2+num);
  if (res_0==res_1) return res_0;
  if (res_0==res_2) return res_0;
  if (res_1==res_2) return res_1;
  return 255;
};
// ------------------Запись EEPROM-----------------------
void alphase_unit::EEPROMwrite(unsigned char num, unsigned char value)
{
  // В EEPROM могут храниться данные только unsigned char
  //if (num>=unit_params_count) return;
  if (EEPROMread(num)==value) return;
  EEPROM.update(num,value);
  EEPROM.update(unit_params_count+num,value);
  EEPROM.update(unit_params_count*2+num,value);
};
// ------------------Ремонт EEPROM-----------------------
void alphase_unit::EEPROMfix()
{
  int num;
  unsigned char res_0, res_1, res_2, total_res;
  for (unsigned short num=0;num<unit_params_count;num++)
  {
    res_0=EEPROM.read(num);
    res_1=EEPROM.read(unit_params_count+num);  
    res_2=EEPROM.read(unit_params_count*2+num);
    if ((res_0==res_1) && (res_1==res_2)) continue;
    if (res_0==res_1) {EEPROM.update(unit_params_count*2+num,res_0);continue;}
    if (res_0==res_2) {EEPROM.update(unit_params_count+num,res_0);continue;}
    if (res_1==res_2) {EEPROM.update(num,res_1);continue;}
    // Если все 3 значения разные
    EEPROM.update(num,0);
    EEPROM.update(unit_params_count+num,0);
    EEPROM.update(unit_params_count*2+num,0);
  }
};
// -----------------------------Работа с полем data------------------
// Получение значения точки
bool alphase_unit::GetPoint(unsigned char point_num)
{
	if (point_num>15) return false;
	if (point_num<8) return GetBitOfByte(data[0], point_num);
	if (point_num>7) return GetBitOfByte(data[1], point_num-8);
}
// Присовение значения точке
unsigned char alphase_unit::SetPoint(unsigned char point_num, bool value)
{
	if (point_num>15) return 0;
	if (point_num<8) return SetBitOfByte(data[0], point_num, value);
	if (point_num>7) return SetBitOfByte(data[1], point_num-8, value);
}
// Получение значения data
unsigned char alphase_unit::GetData(unsigned char index)
{
	if (index>1) return 0;
	return data[index];
}
// Присвоение значения data
void alphase_unit::SetData(unsigned char index, unsigned char value)
{
	if (index>1) return;
	data[index]=value;
}
// Получение бита из байта
unsigned char alphase_unit::GetBitOfByte(unsigned char byte, unsigned char bit_num) //бит справа
{
  return (byte>>bit_num)&(unsigned char)1;
}

//Присвоение бита байту
unsigned char alphase_unit::SetBitOfByte(unsigned char byte, unsigned char bit_num, unsigned char value) //бит справа
{
  if (value==1)
    return ((unsigned char)1<<bit_num)|byte;
  if (value==0)
    return (~((unsigned char)1<<bit_num))&byte;    
}		

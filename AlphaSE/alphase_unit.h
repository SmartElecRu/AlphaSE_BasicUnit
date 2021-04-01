/*
  alphase_unit.h - Library for basic_unit Alpha SE automation system.
  Created by Victor S. Bykov, October 26, 2018.
  Released into the public domain.
*/
#ifndef alphase_unit_h
#define alphase_unit_h
 
#include "Arduino.h"
#include <EEPROM.h> 
#define buff_size 37 // Размер буфера приёмо-передачи (Максимальная длина отправляемого соообщения ModBus 16 регистров=37 байт)
#define unit_params_count 256 // Размерность массива eeprom
#define click_cycles 300 // Количество циклов после которых кнопка считается нажатой

class alphase_unit
{
  public:
		unsigned char serial_active; // Флаг для включения обработки сообщений по RS-485
		unsigned char RS485out_pin; // Порт управления прием/передача RS485
		unsigned char ledWork_pin;  // Светодиод индикации
		unsigned char jmp_pin[8];  // Джампер 0-7. Пины учитываются, если значение порта не равно 0
		//unsigned char jmp1_pin;  // Джампер 1
		//unsigned char jmp2_pin;  // Джампер 2
		//unsigned char jmp3_pin;  // Джампер 3
		//unsigned char jmp4_pin;  // Джампер 4
		//unsigned char jmp5_pin;  // Джампер 5
		//unsigned char jmp6_pin;  // Джампер 6 данный pin джампера не является обязательным и используется, если !==0
		//unsigned char jmp7_pin;  // Джампер 7 данный pin джампера не является обязательным и используется, если !==0

		unsigned char addr; // Адрес модуля в шине
		unsigned char unit_type; //Тип модуля
		unsigned char unit_subtype; //Подтип модуля (при наличии модификаций одного модуля в разных исполнениях)
		unsigned char unit_ver; //Версия модуля
		unsigned char answer_delay; // Задержка перед ответом после приёма команды
		// Массив для загрузки
		unsigned char ee_params_init[unit_params_count];
		unsigned char read_only_params[unit_params_count]; // если в массиве 1, этот параметр только для чтения

		bool on_timer_1s; //один раз в текунду становится true, остальное время false
		
		unsigned char last_cmd_protocol; // Протокол на котором пришла последняя команда 0 - команд не было, 1 - AdNet, 2 - ModBus		
		unsigned char cmd_func_id; //код поступившей команды
		unsigned char cmd_param[6]; //параметры пришедшие во входящей команде ADNet или ModBus
		unsigned char output_data[16]; //данные, отправляемые в ответ за входящую команду ADNet или ModBus
		
		alphase_unit(); // Начальное задение всех переменных
		void start(); // Инициализация пинов согласно начально заданным переменным
		void loop(); //Функция, которая должна выполняться в loop() основной программы

		void SendAnswer_ADNet(); // отправляет ответ по протоколу ADNet
		void SendAnswer_ModBus(); // отправляет ответ по протоколу ModBus

		// Функции работы с EEPROM
		unsigned char EEPROMread(unsigned char num); // Читает байт
		void EEPROMwrite(unsigned char num, unsigned char value); // Записывает байт
		
		bool GetPoint(unsigned char point_num); // Получение значения точки
		unsigned char SetPoint(unsigned char point_num, bool value); // Присовение значения точке
		unsigned char GetData(unsigned char index); // Получение значения data из переменной
		void SetData(unsigned char index, unsigned char value); // Присвоение значения data в переменную
		unsigned char GetBitOfByte(unsigned char byte, unsigned char bit_num); // Получение бита из байта
		unsigned char SetBitOfByte(unsigned char byte, unsigned char bit_num, unsigned char value); //Присвоение бита байту

		void transmit(unsigned char len);		
		unsigned char buff[buff_size]; // буфер входящих/исходящих данных	
  private:
		
		unsigned char temp_uch;
		unsigned char uch_i; // временная переменная типа unsigned char
		unsigned short ush_i;// временная переменная типа unsigned short
		int ledWork_freq; //время между переключениями светодиода: 1000 все хорошо, иначе плохо
		unsigned long last_ledWork; //время последнего переключения светодиода
		unsigned long timer_1s; //Время срабатывания односекундного таймера
		void clear_buff(); // Функция чистит буфер

		void serialEvent(); //проверяем наличие новых данных в интерфейсе шины
		void func_ADNet(); // Функция обработка сообщений RS-485, полученных по протоколу ADNet
		void func_ModBus(); // Функция обработка сообщений RS-485, полученных по протоколу ModBus
		unsigned char GetAddr(); // Возвращает адрес модуля по выставленным Pin
		// Функции работы с EEPROM		
		void EEPROMfix(); //Проверяет на испорченность и восстанавливает
		// Работа с полем data
		unsigned char data[2];  // значение		
		//Вычисление CRC для ADNet и ModBus
		void init_crc16_tab();
		unsigned short update_crc_16( unsigned short crc, char c );
		unsigned short calc_mb_crc(unsigned char *buf, unsigned char count);
		unsigned char  calc_ad_crc(unsigned char *buff_in); // Вычисление crc для ADNet
		
};
 
#endif
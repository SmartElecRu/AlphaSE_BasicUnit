/*
  alphase_unit.h - Library for basic_unit Alpha SE automation system.
  Created by Victor S. Bykov, October 26, 2018.
  Released into the public domain.
*/
#ifndef alphase_unit_h
#define alphase_unit_h
 
#include "Arduino.h"
#include <EEPROM.h> 
#define buff_size 37 // ������ ������ �����-�������� (������������ ����� ������������� ���������� ModBus 16 ���������=37 ����)
#define unit_params_count 256 // ����������� ������� eeprom
#define click_cycles 300 // ���������� ������ ����� ������� ������ ��������� �������

class alphase_unit
{
  public:
		unsigned char serial_active; // ���� ��� ��������� ��������� ��������� �� RS-485
		unsigned char RS485out_pin; // ���� ���������� �����/�������� RS485
		unsigned char ledWork_pin;  // ��������� ���������
		unsigned char jmp_pin[8];  // ������� 0-7. ���� �����������, ���� �������� ����� �� ����� 0
		//unsigned char jmp1_pin;  // ������� 1
		//unsigned char jmp2_pin;  // ������� 2
		//unsigned char jmp3_pin;  // ������� 3
		//unsigned char jmp4_pin;  // ������� 4
		//unsigned char jmp5_pin;  // ������� 5
		//unsigned char jmp6_pin;  // ������� 6 ������ pin �������� �� �������� ������������ � ������������, ���� !==0
		//unsigned char jmp7_pin;  // ������� 7 ������ pin �������� �� �������� ������������ � ������������, ���� !==0

		unsigned char addr; // ����� ������ � ����
		unsigned char unit_type; //��� ������
		unsigned char unit_subtype; //������ ������ (��� ������� ����������� ������ ������ � ������ �����������)
		unsigned char unit_ver; //������ ������
		unsigned char answer_delay; // �������� ����� ������� ����� ����� �������
		// ������ ��� ��������
		unsigned char ee_params_init[unit_params_count];
		unsigned char read_only_params[unit_params_count]; // ���� � ������� 1, ���� �������� ������ ��� ������

		bool on_timer_1s; //���� ��� � ������� ���������� true, ��������� ����� false
		
		unsigned char last_cmd_protocol; // �������� �� ������� ������ ��������� ������� 0 - ������ �� ����, 1 - AdNet, 2 - ModBus		
		unsigned char cmd_func_id; //��� ����������� �������
		unsigned char cmd_param[6]; //��������� ��������� �� �������� ������� ADNet ��� ModBus
		unsigned char output_data[16]; //������, ������������ � ����� �� �������� ������� ADNet ��� ModBus
		
		alphase_unit(); // ��������� ������� ���� ����������
		void start(); // ������������� ����� �������� �������� �������� ����������
		void loop(); //�������, ������� ������ ����������� � loop() �������� ���������

		void SendAnswer_ADNet(); // ���������� ����� �� ��������� ADNet
		void SendAnswer_ModBus(); // ���������� ����� �� ��������� ModBus

		// ������� ������ � EEPROM
		unsigned char EEPROMread(unsigned char num); // ������ ����
		void EEPROMwrite(unsigned char num, unsigned char value); // ���������� ����
		
		bool GetPoint(unsigned char point_num); // ��������� �������� �����
		unsigned char SetPoint(unsigned char point_num, bool value); // ���������� �������� �����
		unsigned char GetData(unsigned char index); // ��������� �������� data �� ����������
		void SetData(unsigned char index, unsigned char value); // ���������� �������� data � ����������
		unsigned char GetBitOfByte(unsigned char byte, unsigned char bit_num); // ��������� ���� �� �����
		unsigned char SetBitOfByte(unsigned char byte, unsigned char bit_num, unsigned char value); //���������� ���� �����

		void transmit(unsigned char len);		
		unsigned char buff[buff_size]; // ����� ��������/��������� ������	
  private:
		
		unsigned char temp_uch;
		unsigned char uch_i; // ��������� ���������� ���� unsigned char
		unsigned short ush_i;// ��������� ���������� ���� unsigned short
		int ledWork_freq; //����� ����� �������������� ����������: 1000 ��� ������, ����� �����
		unsigned long last_ledWork; //����� ���������� ������������ ����������
		unsigned long timer_1s; //����� ������������ �������������� �������
		void clear_buff(); // ������� ������ �����

		void serialEvent(); //��������� ������� ����� ������ � ���������� ����
		void func_ADNet(); // ������� ��������� ��������� RS-485, ���������� �� ��������� ADNet
		void func_ModBus(); // ������� ��������� ��������� RS-485, ���������� �� ��������� ModBus
		unsigned char GetAddr(); // ���������� ����� ������ �� ������������ Pin
		// ������� ������ � EEPROM		
		void EEPROMfix(); //��������� �� ������������� � ���������������
		// ������ � ����� data
		unsigned char data[2];  // ��������		
		//���������� CRC ��� ADNet � ModBus
		void init_crc16_tab();
		unsigned short update_crc_16( unsigned short crc, char c );
		unsigned short calc_mb_crc(unsigned char *buf, unsigned char count);
		unsigned char  calc_ad_crc(unsigned char *buff_in); // ���������� crc ��� ADNet
		
};
 
#endif
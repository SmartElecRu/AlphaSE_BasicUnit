/*
	������ ������ ������������ ��� ��������� �������� ������������ ������� ������������� AlphaSE.
	�������� ������������ ������������� ������ �� ���������� ADNet+ � ModBus RTU.
	���� ��������� ��������� ���������� 22.04.2019�.
*/
#include "alphase_unit.h"

alphase_unit::alphase_unit()
{
	serial_active=0; // ��������� ���������� �� RS485 ������ ������� �� �����
	RS485out_pin = 2; // ���� ���������� �����/�������� RS485
	ledWork_pin = 13;  // ��������� ���������
	jmp_pin[0] = 3;  // ������� 0
	jmp_pin[1] = 4;  // ������� 1
	jmp_pin[2] = 5;  // ������� 2
	jmp_pin[3] = 6;  // ������� 3
	jmp_pin[4] = 7;  // ������� 4
	jmp_pin[5] = 8;  // ������� 5
	jmp_pin[6] = 0;  // ������� 6 ������ pin �������� �� �������� ������������ � ������������, ���� !==0
	jmp_pin[7] = 0;  // ������� 7 ������ pin �������� �� �������� ������������ � ������������, ���� !==0

	for (ush_i=0; ush_i<unit_params_count; ush_i++)
	{
		ee_params_init[ush_i]=0;
		read_only_params[ush_i]=0;
	}
	
	data[0]=0; // ������� ���� data0
	data[1]=0; // ������� ���� data1
	
	read_only_params[0]=1; // ������ ������
	read_only_params[1]=1; // ����� ������
	read_only_params[2]=1; // ��� ������
	read_only_params[3]=1; // ������ ������
	
	unit_type=1; //��� ������. �� ������ ��������� 0. ����� ������ �� ����� ������������ ������������.
	unit_subtype=0; //������ ������ (��� ������� ����������� ������ ������ � ������ �����������)
	unit_ver=1; //������ ������. �� ������ ��������� 0. ����� ������ �� ����� ������������ ������������.	
	on_timer_1s=false; // ���� ������� ����
	last_cmd_protocol=0; // ������ �� �� ������ ��������� �� ���������
	answer_delay=0; // �������� ����� ������� = 0
};

// �������������� ���������� � ��������� ������ ������
void alphase_unit::start()
{
  ledWork_freq=1000; // ������ ������������ LED ������ 1000 - 1 ������������ � �������  
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
	if (jmp6_pin!=0) // ������� 6 �� �������� ������������ � ������������, ���� !==0
		pinMode(jmp6_pin,INPUT);
	if (jmp7_pin!=0) // ������� 7 �� �������� ������������ � ������������, ���� !==0
		pinMode(jmp7_pin,INPUT);
	*/
	digitalWrite(RS485out_pin,LOW);  // ��������� � ����� ���������
	/*
  digitalWrite(jmp0_pin,HIGH); // �������� ������������� �������� 20��� �� +
  digitalWrite(jmp1_pin,HIGH); // �������� ������������� �������� 20��� �� +
  digitalWrite(jmp2_pin,HIGH); // �������� ������������� �������� 20��� �� +
  digitalWrite(jmp3_pin,HIGH); // �������� ������������� �������� 20��� �� +
  digitalWrite(jmp4_pin,HIGH); // �������� ������������� �������� 20��� �� +
  digitalWrite(jmp5_pin,HIGH); // �������� ������������� �������� 20��� �� +
	if (jmp6_pin!=0) // ������� 6 �� �������� ������������ � ������������, ���� !==0
		digitalWrite(jmp6_pin,HIGH); // �������� ������������� �������� 20��� �� +
	if (jmp7_pin!=0) // ������� 7 �� �������� ������������ � ������������, ���� !==0
		digitalWrite(jmp7_pin,HIGH); // �������� ������������� �������� 20��� �� +
	*/	
  // initialize serial:
  Serial.begin(9600);
  delay(3000);  
  
  if (EEPROMread(0)!=ee_params_init[0]) // ���� EEPROM �� �����
  {
    //����������� ��������� ��������
    for (ush_i=0; ush_i<unit_params_count; ush_i++)
    {
			EEPROMwrite(ush_i,ee_params_init[ush_i]);
    };
  };
  EEPROMfix(); // ��������� ������������ �������� � eeprom

  // ������������� CRC ModBus
  init_crc16_tab( );
  clear_buff(); // ������ ����� ������
  
};

void alphase_unit::loop() //�������, ������� ������ ����������� � loop() �������� ���������
{
  addr=GetAddr(); // ��������� ����� ������ �� ����������
	
  if (millis()-last_ledWork>ledWork_freq) // ������� ���������� ������
  {
    last_ledWork=millis();
    digitalWrite(ledWork_pin,!digitalRead(ledWork_pin)); // �������� ���������
  };
  
  if (abs(millis()-timer_1s)>1000) // ���� ������ ������� � �������� ������������ �������
  {
    timer_1s=millis();
    on_timer_1s=true;
  }
	else
	{
		on_timer_1s=false;
	};
	
	serialEvent(); //��������� ������� ����� ������ � ���������� ����
}

// ------------------������� ������ ������-----------------------
void alphase_unit::clear_buff() // ������� ������ �����
{
	// ���� ����� ������� �� �����������
	// �������� ������� ��������� ������ � ������ 11 ������
  for(uch_i=0;uch_i<11;uch_i++) 
    buff[uch_i]=0;
};

// ------------------������� �������� ��������� �� RS-485-----------------------
void alphase_unit::transmit(unsigned char len)
{
  digitalWrite(RS485out_pin,HIGH); // ������������� � ��������
	delay(answer_delay); // �������� ����� �������
  delay(2); // ���� ��� ������, ������������ ���������� ����������� ��������� ������
  Serial.write(buff, len); // �������� �����
  Serial.flush(); // ������� �������� ���� �������
  //delay(5);
  digitalWrite(RS485out_pin,LOW); // ������������� � ����  
	clear_buff(); // ������ �����	
}

// ------------------���� ������-----------------------
//#define ADNet_msg_len 8  // ����� ����� ������� ADNet (8 ��������)
//#define ModBus_read_msg_len_in 8  // ����� ����� ������� Read Holding Registers (������ - 8 ��������)
//#define ModBus_preset_msg_len_in 11  // ����� ����� ������� Preset Multiple Regs (������ - 11 ��������) 

void alphase_unit::serialEvent() {
	// ���� ��������� �� ������ �������
	if (serial_active==0)
	{
		buff[10]=(char)Serial.read();
		return;
	}
	last_cmd_protocol=0; // ���� ���������� �������� ���������� �������
  while (Serial.available()) 
  {
    memcpy(&buff[0],&buff[1],10); // ������� ��� �� 1 ����� ������
    buff[10]=(char)Serial.read(); // �������� ������ � �����

    // ��������� �� ������� ��������� ADNet
    if ((buff[3]==255)&&(buff[4]==255)) // ���� ���� 2 ��������� ����� 255 � 255
    {
      temp_uch=calc_ad_crc(&buff[3]); // ������� CRC � ��������� ADNet
      if (buff[10]==temp_uch) //���� crc ���������
      {
        if (buff[6]==addr) // ���� ����� ������ ���������
        {
					last_cmd_protocol=1; // ������� ������ �� ��������� ADNet
          memcpy(&buff[0],&buff[3],8); // ��������� ������� � ������ ������
          func_ADNet(); // ��������� ������ ������ � ��������� �������
          break; // ������� �� �����
        };
      };
    };
    
    // ��������� �� ������� ��������� ModBus Read
    if ((buff[3]==addr) && (buff[4]==3)) // ���� ������ ������ - �����, ������ - ������� ������
    {
      ush_i=calc_mb_crc(&buff[3],6); //��������� ����������� �����
      memcpy(&buff[0],&ush_i,2);
      if (memcmp(&buff[9],&ush_i,2)==0) // ���� CRC �������
      {
				last_cmd_protocol=2; // ������� ������ �� ��������� ModBus
        memcpy(&buff[0],&buff[3],8); // ��������� ������� � ������ ������
        func_ModBus(); // ��������� �������, ������� �������� �� ���������
        break; // ������� �� �����        
      };
    }

    // ��������� �� ������� ��������� ModBus Preset
    if ((buff[0]==addr) && (buff[1]==16)) // ���� ������ ������ - �����, ������ - ������� ������
    {
      ush_i=calc_mb_crc(&buff[0],9); //��������� ����������� �����
      if (memcmp(&buff[9],&ush_i,2)==0) // ���� CRC �������
      {
				last_cmd_protocol=2;
        func_ModBus(); // ��������� �������, ������� �������� �� ���������
        break; // ������� �� �����        
      };
    }    
  };
};
// ---------------------���������� ������ ADNet--------------------------------
// ������� ����������� ��������� RS-485, ���������� �� ��������� ADNet
void alphase_unit::func_ADNet()
{
  unsigned char uch_i0,uch_i1,i;
  buff[3]=0;
	cmd_func_id=buff[2]; // ��� �������
  switch (buff[2])
  {
  case 0: // ������ ���� � ������ ������
		last_cmd_protocol=0; // ������� �������������� ������������� ������ ����������. � ������� � �������� ��������� �� ����������.
    buff[4]=unit_ver;
    buff[5]=unit_type;
		buff[6]=unit_subtype;
    buff[7]=calc_ad_crc(&buff[0]);
    transmit(8);
    break;
  case 5: // ������� �������� ���������
		cmd_param[0]=buff[5]; // ����� ���������, ������� ����� �������
    break;
  case 6: // �������� �������� ���������
    if((buff[5]>=unit_params_count) || // ���� �������� ������ �������������
		(read_only_params[buff[5]]==1)) // ��� �� read_only
		{
			last_cmd_protocol=0; //true, ���� ��������� ������� AdNet      
			buff[7]=calc_ad_crc(&buff[0]);
			transmit(8);
		}
		else
		{
			cmd_param[0]=buff[5]; // ����� ��������� � ������� ����� ��������� ��������
			cmd_param[1]=buff[4]; // �������� ������� ����� ���������
		}
    break;
  case 10: // ��������� �������
  case 51:
		cmd_param[0]=buff[5]; // ��� ��������. 0 - ��������� �� �����, 1 - ���������� �� �����, 2 - ���������� �����
		cmd_param[1]=buff[4]; // �������� �����
    break;
  case 11: // ������ ��������� ������/�������
  case 50:
		//������� ������� ��������� ������ � �������.
		//������������� � ���� �������� ����������
//    buff[4]=data[0];
//		buff[6]=data[1];
//    buff[7]=calc_ad_crc(&buff[0]);
//    transmit(8);
    break;
  default:
  break;
  }
}
// ������� �������� ��������� ADNet �����������
void alphase_unit::SendAnswer_ADNet()
{
  buff[4]=output_data[0];
  buff[5]=output_data[1];		
  buff[6]=output_data[2];	
  buff[7]=calc_ad_crc(&buff[0]);
	transmit(8);
}
// ------------------ ���������� ������ ModBus --------------------------
// ������� ��������� ��������� RS-485
void alphase_unit::func_ModBus()
{
  unsigned short reg_num;
  unsigned short reg_count;
  reg_num=buff[2]*256+buff[3];  
  reg_count=buff[4]*256+buff[5];  
	
	cmd_func_id=buff[1]; // ��� �������	
	cmd_param[0]=buff[2]; 
	cmd_param[1]=buff[3]; 
	cmd_param[2]=buff[4]; 
	cmd_param[3]=buff[5];
	cmd_param[4]=0; 
	cmd_param[5]=0; 	
	
  if (cmd_func_id==3) // ���� ������� ������
  {
    if (reg_count>16) last_cmd_protocol=0; // �� ����� ���������� ����� 16 ���������
	}
  if (cmd_func_id==16) // ���� ������� ������
  {
    if (reg_count>1) last_cmd_protocol=0; // �� ����� ������������� ����� 1 ��������
    if (buff[6]>2) last_cmd_protocol=0; // ���������� ���� ������ �� ����� ���� ������ 2 
		cmd_param[4]=buff[7]; 
		cmd_param[5]=buff[8]; 	
	}
	return;
}

// ������� �������� ��������� ModBus �����������
void alphase_unit::SendAnswer_ModBus()
{
  unsigned char i;
  unsigned short crc;
  unsigned short reg_num;
  unsigned short reg_count;
  reg_num=cmd_param[0]*256+cmd_param[1];  
  reg_count=cmd_param[2]*256+cmd_param[3];  
  
  if (cmd_func_id==3) // ���� ������� ������
  {
		// ������� �������� ���� ��� ��������� ���������
    //if (reg_count>16) return; // �� ����� ���������� ����� 16 ���������
    buff[2]=reg_count*2; // ���������� ������������� ���� ������ � ������
    for (i=0;i<reg_count;i++)
    {
      buff[3+i*2]=0; // ������ ����� 0 �.�. ��� ��������� unsigned char ee_params_c[reg_num+i] /256;      
      buff[4+i*2]=output_data[i];
    }
    crc=calc_mb_crc(&buff[0],3+reg_count*2);
    memcpy(&buff[3+reg_count*2],&crc,2);
    transmit(5+reg_count*2);
  }

  if (cmd_func_id==16) // ���� ������� ������
  {
		// ������ �������� ���� ��� ��������� ���������
    //if (reg_count>1) return; // �� ����� ������������� ����� 1 ��������
    //if (buff[6]>2) return; // ���������� ���� ������ �� ����� ���� ������ 2 
    crc=calc_mb_crc(&buff[0],6);
    memcpy(&buff[6],&crc,2);
    transmit(8);    
  }
  return;
}

// ------------------���������� ������ ������--------------------------
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
	if (jmp_pin[6]!=0) // ������� 6 �� �������� ������������ � ������������, ���� !==0
		if (digitalRead(jmp_pin[6])==LOW)
			addr+=64;    
	if (jmp_pin[6]!=0) // ������� 7 �� �������� ������������ � ������������, ���� !==0
		if (digitalRead(jmp_pin[7])==LOW)
			addr+=128;    
		
  if (addr>247) return 255; // ������ 248�255 � ��������������� � ��������� ModBus;
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

// ���������� ����������� ����� ModBus
// ���������� ���������� ������ �� ������ ���������
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
// ------------------ ���������� ����������� ����� ��� ������-----------
unsigned char  alphase_unit::calc_ad_crc(unsigned char *buff_in) // ���������� crc ��� ADNet
{
  unsigned char i, crc[3];
  crc[1]=0;
  for(i=2;i<7;i++)
    crc[1]=crc[1]+buff_in[i];
  return crc[1];
}
// ------------------������ EEPROM-----------------------
unsigned char alphase_unit::EEPROMread(unsigned char num)
{
  // � EEPROM ����� ��������� ������ ������ unsigned char
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
// ------------------������ EEPROM-----------------------
void alphase_unit::EEPROMwrite(unsigned char num, unsigned char value)
{
  // � EEPROM ����� ��������� ������ ������ unsigned char
  //if (num>=unit_params_count) return;
  if (EEPROMread(num)==value) return;
  EEPROM.update(num,value);
  EEPROM.update(unit_params_count+num,value);
  EEPROM.update(unit_params_count*2+num,value);
};
// ------------------������ EEPROM-----------------------
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
    // ���� ��� 3 �������� ������
    EEPROM.update(num,0);
    EEPROM.update(unit_params_count+num,0);
    EEPROM.update(unit_params_count*2+num,0);
  }
};
// -----------------------------������ � ����� data------------------
// ��������� �������� �����
bool alphase_unit::GetPoint(unsigned char point_num)
{
	if (point_num>15) return false;
	if (point_num<8) return GetBitOfByte(data[0], point_num);
	if (point_num>7) return GetBitOfByte(data[1], point_num-8);
}
// ���������� �������� �����
unsigned char alphase_unit::SetPoint(unsigned char point_num, bool value)
{
	if (point_num>15) return 0;
	if (point_num<8) return SetBitOfByte(data[0], point_num, value);
	if (point_num>7) return SetBitOfByte(data[1], point_num-8, value);
}
// ��������� �������� data
unsigned char alphase_unit::GetData(unsigned char index)
{
	if (index>1) return 0;
	return data[index];
}
// ���������� �������� data
void alphase_unit::SetData(unsigned char index, unsigned char value)
{
	if (index>1) return;
	data[index]=value;
}
// ��������� ���� �� �����
unsigned char alphase_unit::GetBitOfByte(unsigned char byte, unsigned char bit_num) //��� ������
{
  return (byte>>bit_num)&(unsigned char)1;
}

//���������� ���� �����
unsigned char alphase_unit::SetBitOfByte(unsigned char byte, unsigned char bit_num, unsigned char value) //��� ������
{
  if (value==1)
    return ((unsigned char)1<<bit_num)|byte;
  if (value==0)
    return (~((unsigned char)1<<bit_num))&byte;    
}		

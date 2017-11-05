#include <software.h>
#include <stdlib.h>
#define LCD_ENABLE_PIN PIN_D0
#define LCD_RS_PIN PIN_D1
#define LCD_RW_PIN PIN_D2
#define LCD_DATA4 PIN_D4
#define LCD_DATA5 PIN_D5
#define LCD_DATA6 PIN_D6
#define LCD_DATA7 PIN_D7
#include <lcd.c>
#define RTC_SDA  PIN_C4
#define RTC_SCL  PIN_C3
#include "ds1307.c"

char bufferKeyboard = '0';
enum modos {
   M_HORA, M_DATA, M_CONF_HORA, M_CONF_DATA, M_ALARMES, M_ADD_ALARME,
   M_EDIT_HORA, M_EDIT_DATA, M_SEMANA, M_SUCESSO, M_ADD_HORA_AL,
   M_ADD_SEM_AL, M_LIST_ALARMES
};
signed int modo = 0;
int hora, min, seg, dia, mes, ano, dsem , dsemres;
char sem[4];

char valor1[3] = {"__"};
char valor2[3] = {"__"};
char valor3[3] = {"__"};
int indexTime = 0;
int valor1Int, valor2Int, valor3Int;
signed int valorSemana = 0;

unsigned char meubinSM[8];
unsigned char meubinRT[8];
void intToBin(char *mipt, char valor){
    mipt[0] = (valor & 0b00000001) >> 0; //DOM
    mipt[1] = (valor & 0b00000010) >> 1; //SEG
    mipt[2] = (valor & 0b00000100) >> 2; //TER
    mipt[3] = (valor & 0b00001000) >> 3; //QUA
    mipt[4] = (valor & 0b00010000) >> 4; //QUI
    mipt[5] = (valor & 0b00100000) >> 5; //SEX
    mipt[6] = (valor & 0b01000000) >> 6; //SAB
    mipt[7] = 0; //NULO
    if(mipt[0]) mipt[0] = 'D';
    else mipt[0] = '_';
    if(mipt[1]) mipt[1] = 'S';
    else mipt[1] = '_';
    if(mipt[2]) mipt[2] = 'T';
    else mipt[2] = '_';
    if(mipt[3]) mipt[3] = 'Q';
    else mipt[3] = '_';
    if(mipt[4]) mipt[4] = 'Q';
    else mipt[4] = '_';
    if(mipt[5]) mipt[5] = 'S';
    else mipt[5] = '_';
    if(mipt[6]) mipt[6] = 'S';
    else mipt[6] = '_';
}

char binToInt(char *mipt){
    char valor = 0;
    valor = (mipt[0] << 0) | valor;
    valor = (mipt[1] << 1) | valor;
    valor = (mipt[2] << 2) | valor;
    valor = (mipt[3] << 3) | valor;
    valor = (mipt[4] << 4) | valor;
    valor = (mipt[5] << 5) | valor;
    valor = (mipt[6] << 6) | valor;
    valor = (mipt[7] << 7) | valor;
    return valor;
}

#define QUANT_ALARMES_ATIVO 02
#define INICIO_ADR_ALARME 03

signed int indiceAlarmes = 0;
signed int totalAlarmes = 0;

void inclementQuantAlarme(){
   write_eeprom(QUANT_ALARMES_ATIVO,read_eeprom(QUANT_ALARMES_ATIVO)+1);
}

int editValores(){
   if(indexTime < 2) valor1[indexTime] = bufferKeyboard;
   else if(indexTime < 4) valor2[indexTime-2] = bufferKeyboard;
   else valor3[indexTime-4] = bufferKeyboard;
   indexTime++;
   valor1[2] = '\0';
   valor2[2] = '\0';
   valor3[2] = '\0';
   if(indexTime > 6){
      valor1Int = atoi(valor1);
      valor2Int = atoi(valor2);
      valor3Int = atoi(valor3);
      indexTime = 0;
      return 1;
   }
   else return 0;
}

int editSemana(){
   if(bufferKeyboard == '*'){
      return 1;
   }
   else if(bufferKeyboard != '#') {
      sem[0] = bufferKeyboard;
      sem[1] = "\0";
      dsem = atoi(sem);
      if(dsem >= 7) dsem = 0;
   }
   return 0;
}

void zeraValores(){
   valor1[2] = '\0';
   valor2[2] = '\0';
   valor3[2] = '\0';
   valor1[0] = '_';
   valor2[0] = '_';
   valor3[0] = '_';
   valor1[1] = '_';
   valor2[1] = '_';
   valor3[1] = '_';
}

#INT_RTCC
void  RTCC_isr(void) {
   static int8 contKey;
   static int8 delayKeyboard;
   
   delayKeyboard++;
   if(delayKeyboard > 1) delayKeyboard = 0; 
   
   if(delayKeyboard == 0){
      contKey++;
      if(contKey > 2) contKey = 0;
      switch(contKey){
         case 0:
            output_high(PIN_A0);
            output_low(PIN_A1);
            output_low(PIN_A2);
            if(input_state(PIN_A3)){
               bufferKeyboard = '1';
            }
            else if(input_state(PIN_A4)){
               bufferKeyboard = '4';
            }
            else if(input_state(PIN_A5)){
               bufferKeyboard = '7';
            }
            else if(input_state(PIN_E0)){
               bufferKeyboard = '*';
            }
            break;
         case 1:
            output_high(PIN_A1);
            output_low(PIN_A0);
            output_low(PIN_A2);
            if(input_state(PIN_A3)){
               bufferKeyboard = '2';
            }
            else if(input_state(PIN_A4)){
               bufferKeyboard = '5';
            }
            else if(input_state(PIN_A5)){
               bufferKeyboard = '8';
            }
            else if(input_state(PIN_E0)){
               bufferKeyboard = '0';
            }
            break;
         case 2:
            output_high(PIN_A2);
            output_low(PIN_A1);
            output_low(PIN_A0);
            if(input_state(PIN_A3)){
               bufferKeyboard = '3';
            }
            else if(input_state(PIN_A4)){
               bufferKeyboard = '6';
            }
            else if(input_state(PIN_A5)){
               bufferKeyboard = '9';
            }
            else if(input_state(PIN_E0)){
               bufferKeyboard = '#';
            }
            break;
      }
      if(input_state(PIN_A3) || input_state(PIN_A4) ||
      input_state(PIN_A5) || input_state(PIN_E0)){
      
         //tecla pressionada
         if(modo == M_HORA || modo == M_DATA || modo == M_CONF_HORA || 
         modo == M_CONF_DATA || modo == M_ALARMES || modo == M_ADD_ALARME){
            if(bufferKeyboard == '4') modo--;
            if(bufferKeyboard == '6') modo++;
            if(modo > M_ADD_ALARME) modo = M_HORA;
            if(modo < M_HORA) modo = M_ADD_ALARME;
         }
         
         //Cancelar
         if(modo == M_EDIT_HORA || modo == M_EDIT_DATA || 
         modo == M_SEMANA || modo == M_ADD_HORA_AL || 
         modo == M_ADD_SEM_AL || modo == M_LIST_ALARMES){
            if(bufferKeyboard == '#'){
               modo = M_HORA;
               zeraValores();
               indexTime = 0;
            }
         }
         
         //Entrar
         if(modo == M_CONF_HORA){
            if(bufferKeyboard == '*') modo = M_EDIT_HORA;
         }
         else if(modo == M_CONF_DATA) {
            if(bufferKeyboard == '*') modo = M_EDIT_DATA;
         }
         else if(modo == M_ADD_ALARME){
            if(bufferKeyboard == '*') modo = M_ADD_HORA_AL;
         }
         else if(modo == M_ALARMES){
            if(bufferKeyboard == '*' && totalAlarmes > 0){
               modo = M_LIST_ALARMES;
            }
         }
         else if(modo == M_EDIT_HORA){
            if(editValores()){
               ds1307_get_date(dia,mes,ano,dsem);
               ds1307_set_date_time(dia,mes,ano,dsem,valor1Int,valor2Int,valor3Int);
               zeraValores();
               modo = M_SUCESSO;
            }
         }
         else if(modo == M_EDIT_DATA){
            if(editValores()){
               modo = M_SEMANA;
            }
         }
         else if(modo == M_SEMANA){
            if(editSemana()){
               ds1307_get_time(hora,min,seg);
               ds1307_set_date_time(valor1Int,valor2Int,valor3Int,dsem,hora,min,seg);
               zeraValores();
               modo = M_SUCESSO;
            }
         }
         else if(modo == M_ADD_HORA_AL){
            if(editValores()){
               modo = M_ADD_SEM_AL;
            }
         }
         else if(modo == M_ADD_SEM_AL){
            if(bufferKeyboard == '1'){
               meubinSM[valorSemana] = 1;
               valorSemana++;
            }
            if(bufferKeyboard == '2'){
               meubinSM[valorSemana] = 0;
               valorSemana++;
            }
            if(valorSemana > 6){
               indiceAlarmes = 0;
               while(indiceAlarmes < 60){
                  if(read_eeprom(indiceAlarmes*3+INICIO_ADR_ALARME) == -1){
                     write_eeprom(indiceAlarmes*3+INICIO_ADR_ALARME, valor1Int);
                     write_eeprom(indiceAlarmes*3+INICIO_ADR_ALARME+1, valor2Int);
                     write_eeprom(indiceAlarmes*3+INICIO_ADR_ALARME+2, binToInt(meubinSM));
                     totalAlarmes++;
                     break;
                  }
                  indiceAlarmes++;
               }
               zeraValores();
               modo = M_SUCESSO;
               valorSemana = 0;
            }
         }
         else if(modo == M_LIST_ALARMES){
            if(bufferKeyboard == '4') indiceAlarmes--;
            if(bufferKeyboard == '6') indiceAlarmes++;
            if(indiceAlarmes < 0) indiceAlarmes = totalAlarmes-1;
            if(indiceAlarmes >= totalAlarmes) indiceAlarmes = 0;
            if(bufferKeyboard == '0'){
               int pointer;
               pointer = indiceAlarmes*3+INICIO_ADR_ALARME;
               while(read_eeprom(pointer+3) != -1){
                  write_eeprom(pointer,read_eeprom(pointer+3));
                  write_eeprom(pointer+1,read_eeprom(pointer+4));
                  write_eeprom(pointer+2,read_eeprom(pointer+5));
                  pointer++;
               }
               write_eeprom(pointer,-1);
               write_eeprom(pointer+1,-1);
               write_eeprom(pointer+2,-1);
               totalAlarmes--;
               indiceAlarmes = 0;
               modo = M_SUCESSO;
            }
         }
      }
   }
}

void showHora(){
   printf(lcd_putc,"\f   Hora Atual:");
   ds1307_get_time(hora,min,seg);
   printf(lcd_putc, "\n    %02d:%02d:%02d", hora, min, seg);
}

void showData(){
   printf(lcd_putc,"\f   Data Atual:");
   ds1307_get_day_of_week(sem);
   printf(lcd_putc, "\n %02d/%02d/20%02d %s", dia, mes, ano, sem);
}

void showConfHora(){
   printf(lcd_putc,"\fConfigurar Hora:");
   printf(lcd_putc,  "\n(*) Entrar");
}

void showConfData(){
   printf(lcd_putc,"\fConfigurar Data:");
   printf(lcd_putc,  "\n(*) Entrar");
}

void showAlarmes(){
   printf(lcd_putc,"\fAlarmes:");
   printf(lcd_putc,  "\n(*) Entrar");
}

void editHora(){
   printf(lcd_putc,"\fDigite a hora:");
   printf(lcd_putc,"\n%s:%s:%s",valor1,valor2,valor3);
}

void editData(){
   printf(lcd_putc,"\fDigite a data:");
   printf(lcd_putc,"\n%s/%s/20%s",valor1,valor2,valor3);
}

void addAlarme(){
   printf(lcd_putc,"\f  Add Alarmes:");
   printf(lcd_putc,  "\n(*) Entrar");
}

void showSemana(){
      printf(lcd_putc,"\fEscolha o dia:");
      switch(dsem){
         case 0:
           printf(lcd_putc,"\n7 - Sabado");
            break;
         case 1:
            printf(lcd_putc,"\n1 - Domingo");
            break;
         case 2:
            printf(lcd_putc,"\n2 - Segunda");
            break;
         case 3:
            printf(lcd_putc,"\n3 - Terca");
            break;
         case 4:
            printf(lcd_putc,"\n4 - Quarta");
            break;
         case 5:
            printf(lcd_putc,"\n5 - Quinta");
            break;
         case 6:
            printf(lcd_putc,"\n6 - Sexta");
            break;
      }
}

void addHoraAlarme(){
   printf(lcd_putc,"\fHora do Alarme:");
   printf(lcd_putc,"\n%s:%s:%s",valor1,valor2,valor3);
}

void addSemAlarme(){
   switch(valorSemana){
      case 0:
         printf(lcd_putc,"\fDomingo ?");
         break;
      case 1:
         printf(lcd_putc,"\fSegunda ?");
         break;
      case 2:
         printf(lcd_putc,"\fTerca ?");
         break;
      case 3:
         printf(lcd_putc,"\fQuarta ?");
         break;
      case 4:
         printf(lcd_putc,"\fQuinta ?");
         break;
      case 5:
         printf(lcd_putc,"\fSexta ?");
         break;
     case 6:
        printf(lcd_putc,"\fSabado ?");
        break;
   }
   printf(lcd_putc,"\n(1) Sim (2)Nao");
}

void showSucesso(){
      printf(lcd_putc,"\fAlterado com\nSucesso!");
      delay_ms(700);
      modo = M_HORA;
}

void listaAlarmes(){
   printf(lcd_putc, "\f(%d) %02d:%02d:00", indiceAlarmes+1, read_eeprom(indiceAlarmes*3+INICIO_ADR_ALARME), read_eeprom(indiceAlarmes*3+INICIO_ADR_ALARME+1));
   intToBin(meubinRT,read_eeprom(indiceAlarmes*3+INICIO_ADR_ALARME+2));
   printf(lcd_putc,"\n%c %c %c %c %c %c %c",meubinRT[0],meubinRT[1],meubinRT[2],meubinRT[3],meubinRT[4],meubinRT[5],meubinRT[6]);
}

void main() {
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_256|RTCC_8_bit);      //8,1 ms overflow
   enable_interrupts(INT_RTCC);
   enable_interrupts(GLOBAL);
   lcd_init();

   indiceAlarmes = 0;
   while(indiceAlarmes < 60){
      if(read_eeprom(indiceAlarmes*3+INICIO_ADR_ALARME) != -1){
         totalAlarmes++;
      }
      else break;
      indiceAlarmes++;
   }
   indiceAlarmes = 0;
      
   while(TRUE)
   {
      static int posicaoAlarme;
      //Visualizacao em tempo real
      
      ds1307_get_date(dia,mes,ano,dsemres);
      ds1307_get_time(hora,min,seg);
      
      if(modo == M_HORA) showHora();
      else if(modo == M_DATA) showData();
      else if(modo == M_CONF_HORA) showConfHora();
      else if(modo == M_CONF_DATA) showConfData();
      else if(modo == M_ALARMES) showAlarmes();
      else if(modo == M_EDIT_HORA) editHora();
      else if(modo == M_EDIT_DATA) editData();
      else if(modo == M_SUCESSO) showSucesso();
      else if(modo == M_SEMANA) showSemana();
      else if(modo == M_ADD_ALARME) addAlarme();
      else if(modo == M_ADD_HORA_AL) addHoraAlarme();
      else if(modo == M_ADD_SEM_AL) addSemAlarme(); 
      else if(modo == M_LIST_ALARMES) listaAlarmes();
      
      if(read_eeprom(posicaoAlarme*3+INICIO_ADR_ALARME) == -1) posicaoAlarme = 0;
      if(read_eeprom(posicaoAlarme*3+INICIO_ADR_ALARME) != -1){
         intToBin(meubinRT,read_eeprom(posicaoAlarme*3+INICIO_ADR_ALARME+2));
         if(dsemres == 0) dsemres = 6;
         else dsemres--;
         if(meubinRT[dsemres] != '_'){
            //testa hora e min
            if(read_eeprom(posicaoAlarme*3+INICIO_ADR_ALARME) == hora){
               if(read_eeprom(posicaoAlarme*3+INICIO_ADR_ALARME+1) == min){
                  if(seg > 1){
                     output_high(PIN_E1);
                     printf(lcd_putc,"\f    Alarme \n    Acionado ...");
                     delay_ms(7000);
                     output_low(PIN_E1);
                     printf(lcd_putc,"\f    Espere \n    um pouco ...");
                     delay_ms(53000);
                  }
               }
            }
         }
         posicaoAlarme++;
      }
      delay_ms(200);
   }
}

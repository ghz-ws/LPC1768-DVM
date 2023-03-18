#include "mbed.h"

I2C i2c(P0_0,P0_1);    //SDA,SCL oled anad adc1
I2C i2c2(P0_10,P0_11);   //SDA,SCL adc2
DigitalOut rc1(P2_1);   //relay1
DigitalOut rc2(P2_0);   //relay2
DigitalIn sw1(P0_4);    //sw1
DigitalIn sw2(P0_5);    //sw2

#define intv 10     //interval time

//OLED
const int oled_adr = 0x78;
void oled_init(int adr);     //oled init func
void char_disp(int adr, int8_t position, char data);    //char disp func
void val_disp(int adr, int8_t position, int8_t digit,int val);  //val disp func
void cont(int adr,uint8_t);     //contrast set

//ADC settings
#define res 0.0625          //resolution 2.048/2^16
#define att 10              //volt attenuation
#define sens_R 0.25         //sense R (ohm)
const int adc_adr = 0xd0;
const int sps=0b10;         //0b00->240sps,12bit 0b01->60sps,14bit 0b10->15sps,16bit
char buf[2];                //i2c buf
char set1[1];               //adc1 setting val
char set2[1];               //adc2 setting val

int pga;                //0->x1 1->x2 2->x4 3->x8
int16_t raw_val;        //adc raw val
float val_f;            //raw v/i val (float)
int16_t val;            //raw v/i val (int)
uint16_t disp_val[2];   //display val

int main(){
    thread_sleep_for(100);  //wait for oled power on
    oled_init(oled_adr);
    cont(oled_adr,0xff);
    char_disp(oled_adr,0,'C');
    char_disp(oled_adr,1,'H');
    char_disp(oled_adr,2,'1');
    char_disp(oled_adr,3,':');
    char_disp(oled_adr,0x20,'C');
    char_disp(oled_adr,0x21,'H');
    char_disp(oled_adr,0x22,'2');
    char_disp(oled_adr,0x23,':');

    while (true){
        //adc1
        if(sw1==1){     //VM
            rc1=0;      //relay off
            pga=0;
            if(set1[0]!=(0b1001<<4)+(sps<<2)+pga){
                set1[0]=(0b1001<<4)+(sps<<2)+pga;
                i2c.write(adc_adr,set1,1);
                thread_sleep_for(10);
            }
            i2c.read(adc_adr|1,buf,2);
            raw_val=(buf[0]<<8)+buf[1];
            val_f=raw_val*(att*res);     //mV unit expression
            val=(int16_t)val_f;

            if(raw_val<0){
                char_disp(oled_adr,5,'-');
                val=val*-1;
            }else{
                char_disp(oled_adr,5,'+');
            }
            
            disp_val[0]=val%1000;     //mV
            disp_val[1]=val/1000;     //V
            val_disp(oled_adr,9,3,disp_val[0]);
            val_disp(oled_adr,6,2,disp_val[1]);
            char_disp(oled_adr,8,'.');
            char_disp(oled_adr,12,' ');
            char_disp(oled_adr,13,'V');

        }else if(sw1==0){  //IM
            rc1=1;      //relay on
            pga=3;
            if(set1[0]!=(0b1001<<4)+(sps<<2)+pga){
                set1[0]=(0b1001<<4)+(sps<<2)+pga;
                i2c.write(adc_adr,set1,1);
                thread_sleep_for(10);
            }
            i2c.read(adc_adr|1,buf,2);
            raw_val=(buf[0]<<8)+buf[1];
            val_f=(raw_val*res/8)/sens_R;     //mA unit expression
            val=(int16_t)val_f;

            if(raw_val<0){
                char_disp(oled_adr,5,'-');
                val=val*-1;
            }else{
                char_disp(oled_adr,5,'+');
            }

            disp_val[0]=val%1000;      //mA
            disp_val[1]=val/1000;      //A
            val_disp(oled_adr,8,3,disp_val[0]);
            val_disp(oled_adr,6,1,disp_val[1]);
            char_disp(oled_adr,7,'.');
            char_disp(oled_adr,11,' ');
            char_disp(oled_adr,12,'A');
            char_disp(oled_adr,13,' ');
        }
        //adc1 overflow check
        if(raw_val==32767||raw_val==-32768){
            char_disp(oled_adr,0xf,'!');
        }else{
            char_disp(oled_adr,0xf,' ');
        }

        //adc2
        if(sw2==1){     //VM
            rc2=0;      //relay off
            pga=0;
            if(set2[0]!=(0b1001<<4)+(sps<<2)+pga){
                set2[0]=(0b1001<<4)+(sps<<2)+pga;
                i2c2.write(adc_adr,set2,1);
                thread_sleep_for(10);
            }
            i2c2.read(adc_adr|1,buf,2);
            raw_val=(buf[0]<<8)+buf[1];
            val_f=raw_val*(att*res);     //mV unit expression
            val=(int16_t)val_f;

            if(raw_val<0){
                char_disp(oled_adr,0x20+5,'-');
                val=val*-1;
            }else{
                char_disp(oled_adr,0x20+5,'+');
            }
            
            disp_val[0]=val%1000;     //mV
            disp_val[1]=val/1000;     //V
            val_disp(oled_adr,0x20+9,3,disp_val[0]);
            val_disp(oled_adr,0x20+6,2,disp_val[1]);
            char_disp(oled_adr,0x20+8,'.');
            char_disp(oled_adr,0x20+12,' ');
            char_disp(oled_adr,0x20+13,'V');

        }else if(sw2==0){  //IM
            rc2=1;      //relay on
            pga=3;
            if(set2[0]!=(0b1001<<4)+(sps<<2)+pga){
                set2[0]=(0b1001<<4)+(sps<<2)+pga;
                i2c2.write(adc_adr,set2,1);
                thread_sleep_for(10);
            }
            i2c2.read(adc_adr|1,buf,2);
            raw_val=(buf[0]<<8)+buf[1];
            val_f=(raw_val*res/8)/sens_R;     //mA unit expression
            val=(int16_t)val_f;

            if(raw_val<0){
                char_disp(oled_adr,0x20+5,'-');
                val=val*-1;
            }else{
                char_disp(oled_adr,0x20+5,'+');
            }

            disp_val[0]=val%1000;      //mA
            disp_val[1]=val/1000;      //A
            val_disp(oled_adr,0x20+8,3,disp_val[0]);
            val_disp(oled_adr,0x20+6,1,disp_val[1]);
            char_disp(oled_adr,0x20+7,'.');
            char_disp(oled_adr,0x20+11,' ');
            char_disp(oled_adr,0x20+12,'A');
            char_disp(oled_adr,0x20+13,' ');
        }
        //adc2 overflow check
        if(raw_val==32767||raw_val==-32768){
            char_disp(oled_adr,0x20+0xf,'!');
        }else{
            char_disp(oled_adr,0x20+0xf,' ');
        }
        thread_sleep_for(intv);
    }
}

//oled init func
void oled_init(int adr){
    char oled_data[2];
    oled_data[0] = 0x0;
    oled_data[1]=0x01;           //0x01 clear disp
    i2c.write(adr, oled_data, 2);
    thread_sleep_for(20);
    oled_data[1]=0x02;           //0x02 return home
    i2c.write(adr, oled_data, 2);
    thread_sleep_for(20);
    oled_data[1]=0x0C;           //0x0c disp on
    i2c.write(adr, oled_data, 2);
    thread_sleep_for(20);
    oled_data[1]=0x01;           //0x01 clear disp
    i2c.write(adr, oled_data, 2);
    thread_sleep_for(20);
}

void char_disp(int adr, int8_t position, char data){
    char buf[2];
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    buf[0]=0x40;            //ahr disp cmd
    buf[1]=data;
    i2c.write(adr,buf, 2);
}

//disp val func
void val_disp(int adr, int8_t position, int8_t digit, int val){
    char buf[2];
    char data[4];
    int8_t i;
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    data[3]=0x30+val%10;        //1
    data[2]=0x30+(val/10)%10;   //10
    data[1]=0x30+(val/100)%10;  //100
    data[0]=0x30+(val/1000)%10; //1000
    buf[0]=0x40;
    for(i=0;i<digit;++i){
        buf[1]=data[i+4-digit];
        i2c.write(adr,buf,2);
    }
}

//set contrast func
void cont(int adr,uint8_t val){
    char buf[2];
    buf[0]=0x0;
    buf[1]=0x2a;
    i2c.write(adr,buf,2);
    buf[1]=0x79;    //SD=1
    i2c.write(adr,buf,2);
    buf[1]=0x81;    //contrast set
    i2c.write(adr,buf,2);
    buf[1]=val;    //contrast value
    i2c.write(adr,buf,2);
    buf[1]=0x78;    //SD=0
    i2c.write(adr,buf,2);
    buf[1]=0x28;    //0x2C, 0x28
    i2c.write(adr,buf,2);
}

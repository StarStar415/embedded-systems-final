#include<reg52.h> 
#include <string.h>                       
#define MAX 20
#define DataPort P0
#define KeyPort  P1

// ���y�C�q��ܾ���
sbit LATCH1= P2^2;
sbit LATCH2= P2^3;
sbit SPK   = P2^1; // ��z
// �����K�X�ϥ�
sbit k1 = P2^4;
sbit k2 = P2^5;
sbit k3 = P2^6;
sbit k8 = P2^7;

// �w�q unsigned char �M unsigned int
typedef unsigned char byte;
typedef unsigned int  word;

// ���Φ�C
byte buf[MAX];
byte head = 0;
byte tail = 0;

// �C�q��ܾ�����X�H����� 0-F ���s�X
byte code dofly_DuanMa[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,
		                  	         0x77,0x7c,0x39,0x5e,0x79,0x71}; 
byte code dofly_WeiMa[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f}; 
byte TempData[10]; 
word i;

// �����K�X�������u����
byte code MorseMap[][5] = {
    "01",    "1000",  "1010",  "100",   "0",    "0010",  "110",   "0000", // A-H
    "00",    "0111",  "101",   "0100",  "11",   "10",    "111",   "0110", // I-P
    "1101",  "010",   "000",   "1",     "001",  "0001",  "011",   "1001", // Q-X
    "1011",  "1100"                                                 // Y-Z
};

// ���y�x�}��L
byte KeyScan(void);
byte KeyPro(void);
// �]�w�C�q��ܾ���X�P�s�X
void Display(byte FirstBit,byte Num);


// ��l�� UART
void InitUART  (void){
    SCON  = 0x50;   // SCON: �Ҧ� 1, 8-bit UART, �ϯ౵��  
    TMOD |= 0x20;   // TMOD: timer 1, mode 2, 8-bit ����
    TH1   = 0xFD;   // TH1:  ���˭� 9600 ��C�ǿ�t�v ���� 11.0592MHz  
    TR1   = 1;      // TR1:  timer 1 ���}                         
    EA    = 1;      //���}�`���_
}         

// Delay �ɶ� function�ADelayMs ���� 1ms
void DelayUs2x(unsigned char t){   
    while(--t);
}
void DelayMs(unsigned char t){
    while(t--){
        DelayUs2x(245);
        DelayUs2x(245);
    }
}
// �ŧi Delay function 
void Delay(unsigned int t){
 	while(--t);
}
// �ѽX�����K�X��
void decoder();
void decodeAscii(byte len);
byte MorseToChar(byte* morse, byte length);
// ��z����
void speak(word t){
    for(i=0;i<t;i++)
    {
        DelayUs2x(200); 
        DelayUs2x(200); 
        SPK=!SPK;
        Display(0,8);
    }
}

// �{�b���Ҧ����q�Ʀr�٬O�ѽX�����K�X
bit modeFlags = 0;
// �q�Ʀr�����׬O�_�]�w
bit setAnsNum = 0;
bit f = 0;

byte ansNum[4]; // �s��q�Ʀr����
byte decode[4]; // �s��ѽX�����K�X�����u��
byte guessNum[4]; // �s�� mqtt �q�Ʀr
byte ascii[10]; // �s�� mqtt �ѽX�����K�X
byte cnt=0; 
byte ansCnt=0;
byte decodeCnt=0;

void main (void){
    byte num;
    byte j;
	InitUART();
	ES = 1;                  //���}��f���_
    EA = 1;  
	while (1){
        TR0=0;
        Display(0,8);
        num=KeyPro();
        if(num == 12){
        // �����Ҧ�
	        if(modeFlags == 1){
                modeFlags = 0;
                setAnsNum = 0;
                ansCnt=0;
                i=0;
                for(j=0;j<8;j++)TempData[j]=0;
                TempData[7]=dofly_DuanMa[modeFlags];
	        }
	        else {
                modeFlags = 1;
                decodeCnt = 0;
                i=0;
                for(j=0;j<8;j++)TempData[j]=0;
                TempData[7]=dofly_DuanMa[modeFlags];
	        }  
            DelayMs(100);
	    }

        // �]�w�q�K�X����
        if(modeFlags == 0 && setAnsNum == 0){
            num=KeyPro(); // ���y�x�}��L�ƭ�
            if(num == 15 && ansCnt>=4){
                // �]�w�q�Ʀr�K�X
                setAnsNum = 1;
                for(j=0;j<4;j++)TempData[j]=0;
                TempData[4]=dofly_DuanMa[ansNum[0]];
                TempData[5]=dofly_DuanMa[ansNum[1]];
                TempData[6]=dofly_DuanMa[ansNum[2]];
                TempData[7]=dofly_DuanMa[ansNum[3]];
                i=0;
            }
            if(num>=0 && num<=9){
                bit tmpf = 0;
                if(ansCnt>=4) continue;
                // �P�_�Ʀr�O�_���� �o�X���~����
                for(j = 0 ; j < ansCnt ; j++){
                    if(ansNum[j] == num) {
                        tmpf = 1;
                        break;
                    }
                }
                if(tmpf == 1) {
                    speak(100);
                    DelayMs(100);
                    speak(100);
                    continue;
                }
                if(ansCnt == 0)
                for(i = 0 ; i < 8 ; i++) TempData[i] = 0;
                TempData[ansCnt]=dofly_DuanMa[num];
                ansNum[ansCnt++] = num; 
            }
        }
        else if(modeFlags == 1){
            // ����
            if(k1 == 0){
                decode[decodeCnt++] = 1;
                speak(300);
                SPK=0;
                Delay(300); // �Ѽu��
                while(k1 == 0); // ����}���s
                Delay(300); // �Ѽu��
            }
            // �u��
            if(k2 == 0){
                decode[decodeCnt++] = 0;
                speak(100);
                Delay(300); // �Ѽu��
                while(k2 == 0); // ����}���s
                Delay(300); // �Ѽu��
            }
            // �i��ѽX
            if(k8 == 0){
                decoder();
                Delay(300); // �Ѽu��
                while(k8 == 0); // ����}���s
                Delay(300); // �Ѽu��
            }            
        }
	}
}

// �ѽX�����u�� �óz�L UART �ǰe�� ESP32
void decoder(void) {
    byte morseChar = MorseToChar(decode, decodeCnt);
    decodeCnt = 0;

    if (morseChar != 0xFF) {
        SBUF = morseChar;
        while (TI == 0);
        TI = 0;
        SBUF = '\r';
        while (TI == 0);
        TI = 0;
        SBUF = '\n';
        while (TI == 0);
        TI = 0;
    }
}

// �������u���æ^�ǹ������r�� �S���^�� 0xFF
byte MorseToChar(byte* morse, byte length) {
    byte i, j, match;
    for ( i = 0; i < sizeof(MorseMap) / sizeof(MorseMap[0]); i++) {
        if (strlen(MorseMap[i]) == length) {
            match = 1;
            for ( j = 0; j < length; j++) {
                if ((MorseMap[i][j] - '0') != morse[j]) {
                    match = 0;
                    break;
                }
            }
            if (match) return 'A' + i;
        }
        
    }
    return 0xFF; 
}

// �ѽX MQTT�ǰe�� ESP32 �A�z�L UART �ǰe�L�Ӫ� ASCII �������K�X�o���e�{
void decodeAscii(byte len) {
    byte morseCode[5];
    byte length, i, j, k;
    for(i = 0 ; i < len  ; i++){
        for (j = 0; j < sizeof(MorseMap) / sizeof(MorseMap[0]); j++) {
            if ((ascii[i] - 'A') == j) {
                strcpy(morseCode, MorseMap[j]);
                length = strlen(morseCode);
                for ( k = 0; k < length; k++) {
                    if (morseCode[k] == '0') {
                        speak(100);
                    } else {
                        speak(300);
                    }
                    SPK = 0;
                    DelayMs(200);
                }
                DelayMs(200);
                DelayMs(200);
            }
        }
    }
}

// �q�Ʀr�C��
void getGuessNumAns(void) {
    byte a = 0; 
    byte b = 0; 
    byte i, j;
    byte ansUsed[4] = {0}; 
    byte guessUsed[4] = {0}; 

    // �p�� A ���ƶq
    for (i = 0; i < 4; i++) {
        if ((guessNum[i]-'0') == ansNum[i]) {
            a++;
            ansUsed[i] = 1;
            guessUsed[i] = 1;
        }
    }

    // �p�� B ���ƶq
    for (i = 0; i < 4; i++) {
        if (guessUsed[i] == 0) {
            for (j = 0; j < 4; j++) {
                if (ansUsed[j] == 0 && guessNum[i]-'0' == ansNum[j]) {
                    b++;
                    ansUsed[j] = 1;
                    break;
                }
            }
        }
    }

    // �C�q��ܾ���ܴX A �X B
    TempData[4] = dofly_DuanMa[a];
    TempData[5] = dofly_DuanMa[10]; 
    TempData[6] = dofly_DuanMa[b];
    TempData[7] = dofly_DuanMa[11]; 
    
    // �^�ǲq�Ʀr���G�� ESP32
    SBUF = a + '0';
    while (TI == 0);
    TI = 0;
    SBUF = 'A';
    while (TI == 0);
    TI = 0;
    SBUF = b + '0';
    while (TI == 0);
    TI = 0;
    SBUF = 'B';
    while (TI == 0);
    TI = 0;
    SBUF = '\r';
    while (TI == 0);
    TI = 0;
    SBUF = '\n';
    while (TI == 0);
    TI = 0;
    
    // �q�� ��� SUCCESS �õo�X����
    if (a == 4){
        setAnsNum = 0;
        for(i=0;i<8;i++)TempData[i]=0;
        ansCnt=0;
        // ��� success
        TempData[0] = 0x6D; // S
        TempData[1] = 0x3E; // U
        TempData[2] = 0x39; // C
        TempData[3] = 0x39; // C
        TempData[4] = 0x79; // E
        TempData[5] = 0x6D; // S
        TempData[6] = 0x6D; // S
        speak(100);
        DelayMs(100);
        speak(100);
        DelayMs(500);
        speak(100);
        DelayMs(100);
        speak(100);
    }
}

void getHintNumAns(void) {
    byte a = 0; 
    byte b = 0; 
    byte i;
    byte ansUsed[4] = {0}; 
    byte guessUsed[4] = {0}; 

    // ���ܥ\�� ��� A ����m���ǬO���T��
    for (i = 0; i < 4; i++) {
        if ((guessNum[i]-'0') == ansNum[i]) {
            a++;
            TempData[i] = dofly_DuanMa[guessNum[i]-'0'];
            SBUF = guessNum[i];
            while (TI == 0);
            TI = 0;
        }
        else{
            TempData[i] = 0x40;
             SBUF = '-';
            while (TI == 0);
            TI = 0;
        }
    }
    
    SBUF = '\r';
    while (TI == 0);
    TI = 0;
    SBUF = '\n';
    while (TI == 0);
    TI = 0;

    for(i=4;i<8;i++)TempData[i]=0;
}

// �����T��
void UART_SER (void) interrupt 4 {
    byte Temp;       
    
    // �p�G RI==1��ܦ���ưe�i��
    if(RI){
        RI=0;                   
        Temp=SBUF;// �q SBUF ������Ʀs�b buffer ��  
        buf[head] = Temp;
        // �q�Ʀr�Ҧ�
        if(modeFlags == 0 && setAnsNum == 1){
            // �����쪺�r��������r���A�N cnt �]�� 0 ���ݤU������
            if (Temp == '\n'){
                cnt = 0;
            } 
            else if(Temp == '\r'){
                // �p�G���S�����~�άO���� �h�i��q�Ʀr
                if(f == 0) getGuessNumAns();
                f = 0;
                cnt = 0;
            }
            // ���ܥ\��
            else if(Temp == '*'){
                f = 1;
                getHintNumAns();
            }
            // �N���쪺�r���x�s�ó]�w��C�q��ܾ��W���
            else if(Temp >= '0' && Temp <= '9') {
                guessNum[cnt] = Temp;
                TempData[cnt] = dofly_DuanMa[guessNum[cnt]-'0'];
                cnt++;
            }
            // ��������~�r�� ��� error �õo�X����
            else{
                f=1;
                for(i=0;i<8;i++)TempData[i]=0;
                // ��� error
                TempData[0]=0x79;
                TempData[1]=0x50;
                TempData[2]=0x50;
                TempData[3]=0x5c;
                TempData[4]=0x50;
                speak(100);
                DelayMs(100);
                speak(100);
                SPK=0;
            }
        }
        // �����K�X�Ҧ�
        if(modeFlags == 1){
            // �����쪺�r��������r���A�N cnt �]�� 0 ���ݤU������
            if (Temp == '\n'){
                cnt = 0;
            } 
            else if(Temp == '\r'){
                // �p�G���S�����~�άO���� �h�i�� ascii �ѽX
                if(f==0)decodeAscii(cnt);
                f = 0;
                cnt = 0;
            }
            // �N���쪺�r���x�s�ó]�w��C�q��ܾ��W���
            else if((Temp >= 'A' && Temp <= 'Z') || (Temp >= 'a' && Temp <= 'z')) {
                for(i=0;i<7;i++)TempData[i]=0;
                ascii[cnt] = Temp;
                cnt++;
            }
            // ��������~�r�� ��� error �õo�X����
            else  {
                f=1;
                for(i=0;i<8;i++)TempData[i]=0;
                // ��� error
                TempData[0]=0x79;
                TempData[1]=0x50;
                TempData[2]=0x50;
                TempData[3]=0x5c;
                TempData[4]=0x50;
                speak(100);
                DelayMs(100);
                speak(100);
                SPK=0;
            }
        }
        // ���Φ�C
        head++;
        if (head == MAX) head = 0;
    }
} 

// ��ܤC�q��ܾ�
void Display(byte FirstBit,byte Num){
    static byte i=0;

    DataPort=0;   
    LATCH1=1;     
    LATCH1=0;

    DataPort=dofly_WeiMa[i+FirstBit]; 
    LATCH2=1;     
    LATCH2=0;

    DataPort=TempData[i]; 
    LATCH1=1;    
    LATCH1=0;	  	  
      
    i++;
    if(i==Num)
    i=0;
}

// ���y�x�}��L
unsigned char KeyScan(void){
    unsigned char Val;
    KeyPort=0xf0;
    if(KeyPort!=0xf0){
        DelayMs(10); 
        if(KeyPort!=0xf0){          
            KeyPort=0xfe; 
            if(KeyPort!=0xfe){
                Val=KeyPort&0xf0;
                Val+=0x0e;
                while(KeyPort!=0xfe);
                DelayMs(10); 
                while(KeyPort!=0xfe);
                return Val;
            }
            KeyPort=0xfd;
            if(KeyPort!=0xfd){
                Val=KeyPort&0xf0;
                Val+=0x0d;
                while(KeyPort!=0xfd);
                DelayMs(10); 
                while(KeyPort!=0xfd);
                return Val;
            }
            KeyPort=0xfb; 
            if(KeyPort!=0xfb){
                Val=KeyPort&0xf0;
                Val+=0x0b;
                while(KeyPort!=0xfb);
                DelayMs(10); 
                while(KeyPort!=0xfb);
                return Val;
            }
            KeyPort=0xf7;
            if(KeyPort!=0xf7){
                Val=KeyPort&0xf0;
                Val+=0x07;
                while(KeyPort!=0xf7);
                DelayMs(10);
                while(KeyPort!=0xf7);
                return Val;
            }
        }
    }
    return 0xff;
}

// �^�ǯx�}��L���y�n�Ψ쪺�ƭ�
byte KeyPro(void){
    switch(KeyScan()){
        case 0x7e:return 0;break; // 0
        case 0x7d:return 1;break; // 1
        case 0x7b:return 2;break; // 2
        case 0x77:return 3;break; // 3
        case 0xbe:return 4;break; // 4
        case 0xbd:return 5;break; // 5
        case 0xbb:return 6;break; // 6
        case 0xb7:return 7;break; // 7
        case 0xde:return 8;break; // 8
        case 0xdd:return 9;break; // 9
        case 0xdb:return 10;break; // A
        case 0xd7:return 11;break; // B
        case 0xee:return 12;break; // C
        case 0xed:return 13;break; // D
        case 0xeb:return 14;break; // E
        case 0xe7:return 15;break; // F
        default:return 0xff;break; // Invalid key
    }
}

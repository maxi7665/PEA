#include "stm32f10x.h"

// keypad button indexes
#define KEYPAD_1 0
#define KEYPAD_2 1
#define KEYPAD_3 2
#define KEYPAD_4 4
#define KEYPAD_5 5
#define KEYPAD_6 6
#define KEYPAD_7 8
#define KEYPAD_8 9
#define KEYPAD_9 10
#define KEYPAD_0 13
#define KEYPAD_PLUS 3
#define KEYPAD_MINUS 7
#define KEYPAD_DIV 18
#define KEYPAD_LN 14
#define KEYPAD_MUL 11
#define KEYPAD_EQ 19
#define KEYPAD_CLEAR 15
#define KEYPAD_OPEN_BR 16
#define KEYPAD_CLOSE_BR 17
#define KEYPAD_DOT 12


// app states
#define STATE_START 0 //user hello
#define STATE_INPUT 1 //wait user input
#define STATE_SEND_COMMAND 2 // send command to server
#define STATE_WAIT_RESULT 3 // wait result from server
#define STATE_SHOW_RESULT 4 // result printed to screen
#define STATE_ERROR 5 // error state
#define STATE_PRINT_HISTORY 6 // print history from server

char* keypad_chars[20] = {"1", "2", "3", "+", "4", "5", "6", "-", "7", "8", "9", "*", ".", "0", "ln", "C", "(", ")", "/", "="};
	
// string buffers for UART send-receive
#define CMD_BUFFER_LEN 80
#define RES_BUFFER_LEN 80
	
char cmd_buffer[CMD_BUFFER_LEN];
char cmd_buffer_pos = 0;	
char res_buffer[RES_BUFFER_LEN];

// row addresses in lcd screen
const uint8_t row_addresses[] = {0x00, 0x40, 0x14, 0x54};
uint8_t cur_row = 0, cur_col = 0;

uint8_t app_state = STATE_START;

void delay_us(uint32_t us) {
    // ??? 8 ??? ???????? ??????????? ???, ????? ?????????? ??????? us
    uint32_t count = us * 2; 
    while(count--) {
        __NOP();
    }
}

void LCD_Pulse(void) {
    GPIOA->BSRR = GPIO_BSRR_BS1; // EN = 1
    delay_us(5);                 // ????? ????????? "Enable" (??? 450??)
    GPIOA->BSRR = GPIO_BSRR_BR1; // EN = 0
    delay_us(40);                // ????? ?? ?????????? ???????
}

void LCD_SendNibble(uint8_t nibble, uint8_t rs) {
    if(rs) GPIOA->BSRR = GPIO_BSRR_BS0; // RS = 1 (??????)
    else   GPIOA->BSRR = GPIO_BSRR_BR0; // RS = 0 (???????)
    
    // ??????? PA2-PA5 ? ??????? 4 ???? ??????
    GPIOA->ODR = (GPIOA->ODR & ~(0xF << 2)) | ((nibble & 0x0F) << 2);
    LCD_Pulse();
}

void LCD_Cmd(uint8_t cmd) {
    LCD_SendNibble(cmd >> 4, 0);   // ??????? ????????
    LCD_SendNibble(cmd & 0x0F, 0); // ??????? ????????
    if (cmd == 0x01 || cmd == 0x02) delay_us(2000); // ??????? ???????
}

void LCD_Data(uint8_t data) {
    LCD_SendNibble(data >> 4, 1);
    LCD_SendNibble(data & 0x0F, 1);
}

void LCD_SetCursor(uint8_t col, uint8_t row) {
    cur_row = row % 4;
    cur_col = col % 20;
    LCD_Cmd(0x80 | (cur_col + row_addresses[cur_row]));
}

void LCD_Init(void) {
    // 1. ???????? ???????????? ????? A
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // 2. ????????? PA0-PA5 ??? Output Push-Pull (10MHz)
    // ??????? ???? CNF ? MODE ??? ????? 0-5
    GPIOA->CRL &= ~(0x00FFFFFF); 
    // ?????? MODE=01 (10MHz), CNF=00 (Push-Pull) -> 0x1 ?? ?????? ???
    GPIOA->CRL |=  (0x00111111); 

    // 3. ????????? ????????????? 4-??? (?? ????????)
    delay_us(20000); // ???????? ????????? ??????? (>15??)
    LCD_SendNibble(0x03, 0); delay_us(5000);
    LCD_SendNibble(0x03, 0); delay_us(200);
    LCD_SendNibble(0x03, 0);
    LCD_SendNibble(0x02, 0); // ??????? ?? 4-?????? ?????????

    // 4. ????????? ??????????
    LCD_Cmd(0x28); // 4-???, 2(4) ??????, ????? 5x8
    LCD_Cmd(0x0C); // ??????? ???, ?????? ????
    LCD_Cmd(0x06); // ????-????????? ??????
    LCD_Cmd(0x01); // ??????? ??????
}

void LCD_Print(const char* str) {
    while(*str) {
        if (cur_col >= 20) {
            cur_col = 0;
            cur_row = (cur_row + 1) % 4;
            LCD_SetCursor(cur_col, cur_row);
        }
        LCD_Data(*str++);
        cur_col++;
    }
}

void LCD_Backspace(uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        if (cur_col == 0) {
            cur_row = (cur_row == 0) ? 3 : cur_row - 1;
            cur_col = 19;
        } else {
            cur_col--;
        }
        LCD_SetCursor(cur_col, cur_row);
        LCD_Data(' '); 
        LCD_SetCursor(cur_col, cur_row);
    }
}

// clean LCD
void LCD_Clear() {
		LCD_Cmd(0x01); 
		cur_row = 0;
		cur_col = 0;
		LCD_SetCursor(cur_col, cur_row);
}

void Keypad_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    GPIOB->CRL &= ~0xFFFFFFFF; 
    GPIOB->CRL |=  0x88833333; 

    GPIOB->CRH &= ~0x0000000F; 
    GPIOB->CRH |=  0x00000008; 

    GPIOB->ODR |= (0xF << 5); 
}

//uint8_t Scan_Keypad(void) {
//    for (int r = 0; r < 5; r++) {
//        GPIOB->ODR = (GPIOB->ODR | 0x1F) & ~(1 << r);
//        
//        uint16_t cols = (GPIOB->IDR >> 5) & 0x0F;
//        
//        if (cols != 0x0F) { 
//            return (r * 4) + cols; 
//        }
//    }
//    return 0xFF; 
//}

uint8_t Scan_Keypad(void) {
    for (uint8_t r = 0; r < 5; r++) {
        GPIOB->ODR = (GPIOB->ODR | 0x1F) & ~(1 << r); // ?????????? ?????? r
        uint16_t cols = (GPIOB->IDR >> 5) & 0x0F;     // ?????? ??????? PB5-PB8
        
        if (cols != 0x0F) { // ???? ?????-?? ?????? ??????
            for (uint8_t c = 0; c < 4; c++) {
                if (!(cols & (1 << c))) { // ???????, ? ????? ?????? ???? "0"
                    return (r * 4) + c;   // ?????????? ?????? (0-19)
                }
            }
        }
    }
    return 0xFF; // ?????? ?? ??????
}

void Key_To_Op(uint8_t key, char* buffer) {
	uint8_t row = key / 4;
	uint8_t col = key % 4;
	
	char row_c = row + 48;
	char col_c = col + 48;
	
	buffer[0] = row_c;
	buffer[1] = col_c;
}

char* Get_Key_Char(uint8_t index) {
	return keypad_chars[index];
}

// clean buffer
void Clean_Cmd_Buffer() {
		for (int i = 0; i < 80; i++) {	
			cmd_buffer[i] = 0;
		}	
		cmd_buffer_pos = 0;
}

// add data to buffer
void Copy_To_Cmd_Buffer(char *str) {
	while (*str) {
		if (cmd_buffer_pos < 80) {
				cmd_buffer[cmd_buffer_pos++] = (*str++);
		}
	}
}


		
#define UART_BAUD_9600 0x341
		
void UART3_Init(void) {
    // ???????? ????????????
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN);
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART3EN);

    // PB10 (TX) - Alt. Function Push-Pull
    MODIFY_REG(GPIOB->CRH, (GPIO_CRH_MODE10 | GPIO_CRH_CNF10), 
                           (GPIO_CRH_MODE10_1 | GPIO_CRH_MODE10_0 | GPIO_CRH_CNF10_1));

    // PB11 (RX) - Input Floating
    MODIFY_REG(GPIOB->CRH, (GPIO_CRH_MODE11 | GPIO_CRH_CNF11), 
                           (GPIO_CRH_CNF11_0));

    // ???????? 9600 ??? 8 ???
    WRITE_REG(USART3->BRR, UART_BAUD_9600);

    // ????????? ??????, ??????????? ? ?????????
    SET_BIT(USART3->CR1, USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
}

void UART3_SendChar(char c) {
    while (READ_BIT(USART3->SR, USART_SR_TXE) == 0);
    WRITE_REG(USART3->DR, c);
}

void UART3_SendString(const char* str) {
    while (*str) UART3_SendChar(*str++);
}

char UART3_ReceiveChar(void) {
    while (READ_BIT(USART3->SR, USART_SR_RXNE) == 0);
    return (char)(USART3->DR & 0xFF);
}

// check if uart3 has income data
char UART3_HasIncomeData() {
		if (READ_BIT(USART3->SR, USART_SR_RXNE) == 0) {
			return 0;
		} 
		else {
			return 1;
		}
}

void UART3_SendLine(const char* str) {
		UART3_SendString(str);
		UART3_SendString("\r\n");
}

// get line from uart3
void UART3_ReceiveLine(char* buffer, uint16_t buffer_size) {
		char prev_sym = 0;
		while (1) {
				char sym = UART3_ReceiveChar();
			
				if (sym != '\r' && sym != '\n') {
						*buffer++=sym;
						if (--buffer_size < 1) break;
				}
				
				if (sym == '\n' && prev_sym == '\r') {
						break;
				}
				
				prev_sym = sym;
		}
		
		while (buffer_size--) *buffer++='\0';
}


// calc expression and get result in buffer
uint8_t CalculateExpr(
		char* expr, 
		uint16_t expr_size, 
		char* res_buffer, 
		uint16_t buf_size) {
			
	UART3_SendLine(expr);
	UART3_ReceiveLine(res_buffer, buf_size);
	
	return 0;
}


int main(void) {
    LCD_Init();
		Keypad_Init();
		UART3_Init();
	
		LCD_Print("CALCULATOR :D");
	
		char* ops = "00";				
    
    while(1) {
				uint8_t key = Scan_Keypad();
			
				if (key != 0xFF) {
						while (key == Scan_Keypad());										
					
						switch (key) {
							
							case KEYPAD_CLEAR:
								
								LCD_Clear();
								Clean_Cmd_Buffer();
							
								break;
							
							case KEYPAD_EQ:
																
								if (cmd_buffer_pos > 0) {	
									app_state = STATE_SEND_COMMAND;		
									LCD_Clear();										
									uint8_t res = CalculateExpr(cmd_buffer, cmd_buffer_pos, res_buffer, RES_BUFFER_LEN);
									Clean_Cmd_Buffer();									
								
									if (res != 0) {
										app_state = STATE_ERROR;									
										LCD_Print("ERROR");
									}
									else {																				
										app_state = STATE_SHOW_RESULT;
										LCD_Print(res_buffer);
									}
								}
								
								break;
							
							default:
								
								if (app_state != STATE_INPUT) {
									LCD_Clear();
									app_state = STATE_INPUT;
								}
								
								ops = Get_Key_Char(key);					
								LCD_Print(ops);			
								Copy_To_Cmd_Buffer(ops);
						}
					
						delay_us(5000);
				}
				
				if (UART3_HasIncomeData()) {
						app_state = STATE_PRINT_HISTORY;
						UART3_ReceiveLine(res_buffer, RES_BUFFER_LEN);
						LCD_Clear();
						Clean_Cmd_Buffer();
						LCD_Print(res_buffer);						
				}
		}
}

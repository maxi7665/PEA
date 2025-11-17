int main () {
	volatile unsigned char a1 = 0x35; 
	volatile unsigned char b1 = 0x35; 
	volatile unsigned char c1 = 0x35; 
	volatile unsigned short a2 = 0x3535; 
	volatile unsigned short b2 = 0x3535; 
	volatile unsigned short c2 = 0x3535; 
	volatile unsigned int a4 = 0x35353535;
	volatile unsigned int b4 = 0x35353535;
	volatile unsigned int c4 = 0x35353535;
	volatile unsigned long long a8 = 0x3535353535353535; 
	volatile unsigned long long b8 = 0x3535353535353535;
	volatile unsigned long long c8 = 0x3535353535353535;
	volatile char name1[] = "Maxim";
	volatile char name2[] = "Bystrov";
	volatile char name3[] = "4131z";
	for(;;){}
	return 0;
}
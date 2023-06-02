#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

const int TONALITY = 160;
const int SOUND_DURATION_MILISECONDS = 30;
int MAX_TONALITY = 9;
int SPEED = 2000;

void SetTonality(int tonalityHerz);
void ReadStatusWords(void);
void EnableSound();
void DisableSound();
short HerzToShort(int);
void ChannelsKd();

void PlaySong(const char* tabs);
void SingNote(int note, int londnes);

void main()
{
	char c;
	const char* CrazyTrain = "F23 F23 C33 F23 d33 F23 C33 F23 b23 a23 G23 a23 b23 a23 G23 e23 F23 F23 C33 F23 d33 F23 C33 F23 b23 a23 G23 a23 b23 a23 G23 e23 F23 F23 C33 F23 d33 F23 C33 F23 b23 a23 G23 a23 b23 a23 G23 e23 F23 F23 C33 F23 d33 F23 C33 F23 ";
	const char* FuneralMarch = "a22 a23.a24 a22 c33.b25 6c36 b23.a24 a23.a24 a21 a22 a23.a24 a22 e33.d35 6d36 d33.c34 c33.c34 c31 a33 g33 f33.e34 e32 c32 a33 g33 f33.e34 e32 c32 a22 a23.a24 a22 c33.b25 6c36 b23.a24 a23.a24 a21 ";
	const char* ImperialMarch = "g32 g32 g32 D33.A34 g32 D33.A34 g31 d42 d42 d42 D43.A34 F32 D33.A34 g31 g42 g33.g34 g42 F43.f44 e44 D44 e43 3G33 C42 c43.b34 A34 a34 A33 3D33 F32 D33.F34 A32 g33.A34 d41 g42 g33.g34 g42 F43.f44 e44 D44 e43 3G33 C42 c43.b34 A34 g34 A33 3D33 F32 D33.A34 g32 D33. A34 g31 ";
	const char* MortalKombat = "a33 a33 c43 a33 d43 a33 ^d43 c43 c43 e43 c43 g43 c43 e43 c43 g33 g33 b33 g33 c43 g33 d43 ^c43 f33 f33 a33 f33 b33 f33 c43 ^b33 a33 a33 c43 a33 d43 a33 ^d43 c43 c43 e43 c43 g43 c43 e43 c43 g33 g33 b33 g33 c43 g33 d43 ^c43 f33 f33 a33 f33 b33 f33 c43 ^b33 a33 3a33 3a33 3a33 3g33 c43 a33 3a33 3a33 3a33 3g33 ^e33 a33 3a33 3a33 3a33 3g33 c43 a33 3a33 3a33 a34 a33 a34 a33 3a33 3a33 3a33 3a33 3g33 c43 a33 3a33 3a33 3a33 3g33 ^e33 a33 3a33 3a33 3a33 3g33 c43 a33 3a33 3a33 a34 a33 a34 a33 3a34 e43 a34 c43 a34 ^A33 a34 c43 a34 ^A34 g33 a34 e43 a34 c43 a34 ^A33 a34 c43 a34 ^A34 g33 a34 e43 a34 c43 a34 ^A33 a34 c43 a34 ^A34 g33 a34 e43 a34 c43 g34 g33 g34 a32 2a34 e43 a34 c43 a34 ^A33 a34 c43 a34 ^A34 g33 a34 e43 a34 c43 a34 ^A33 a34 c43 a34 ^A34 g33 a34 e43 a34 c43 a34 ^A33 a34 c43 a34 ^A34 g33 a33 a33 a33 a33 ";
	//const char* MAMMAMIA = "G22 D32 G22 D32 e31 D32 e33 e33 e33 G32 e32 G33 e32 a31 G33 G33 G33 G33 G33 G22 D32 G22 D32 e31 D32 e33 e33 e33 G32 e32 G33 e32 a31 G33 G33 G33 G33 G33 3D33 D33 D33 D33 e32 C32 D33 D33 D33 D33 e33 D32.D33 D33 D33 e33 D32 C32 D33 D33 D33 e33 D33 D32.D33 D33 D33 D33 e32 C32 D33 D33 D33 D33 e33 D32.D33 D33 D33 e33 D32 C32 D33 D33 D33 e33 D33 D32 D33 D33 D33 D33 e32 C32 D33 D33 D33 D33 e33 D32.D33 D33 D33 e33 D32 C32 D33 D33 D33 e33 D33 D32.D33 D33 D33 D33 e32 C32 D33 D33 D33 D33 e33 D32.D33 D33 D33 e33 D32 C32 D33 D33 D33 e33 D33 D32 ";
	//const char* Unravel = "A43 c52 A42 a43 g42 c52 A42 a42 g42 g43 f42.f43 D42 f43 d43 ^d40 123d33 d32 d33 d32 d43 d41.23A33 a32 a33 a32 A33 A31 a43 a42 A43 A43 A33 c42 A33 a33 g33 3c52 A42 a42 g42 g43 f43 2f43 D42 f43 d41.2d43 d42 d43 d42 d53 d53 d43 d42 d43 d42 d53 d53 A43 a42 a43 a42 A43 A43 g44 g44 4g44 4g44 g44 4g44 g44 4g44 4g44 g44 4d44 g34 g34 a34 g34 g34 A34 a34 d44 g34 g34 a34 g34 g34 A34 a34 ";
	const char* Unravel_1 = "A43 c52 A42 a43 g42 c52 A42 a42 g42 g43 f43.34D43 D43 f43 d42 213d43 d42 d43 d42 d53 d52.313A43 a42 a43 a42 A43 A42.313A43 c52 A42 a43 g43.4c52 A42 a42 g43.4g43 f43.43D43 D42 f43 d42.13d43 d42 d43 d42 d53 d52. 313A43 a42 a43 a42 A43 A42 A34 A33 A33 A34 A33 A34 A33 A34 A33 3d44 d34 g34 c44 d34 g34 A34 g34 d44 d34 g34 c44 d34 g34 c43 d54 d43 c53 d44 A43 d54 d43 c54 A42 d44 A43 d44 g43 d43 d34 g34 d44 d34 g34 c43.d44 A43 d44 g43 d43 d33 A43.a43.d44 A43 d44 g43 d43 d34 g34 d44 d34 g34 ";
	const char* Unravel_2 = "c44.d44 A43 d44 g43 d43 d33 A43.a43.a34 A34 4g33.c43 a34 c44 4g33 4a43 A43.A43.A42.A43 d53 d53.c53.c52.3A43 c54.A43.a42 33d43 c42 3f43 f43 d44 d43.d43 c43 d44 d43 4f43 f43 d44 d43.d43 c43 d44 d43 4f43 f43 d44 d43.d43 c43.d43.d43.433d43 d52 c53 c53 d53.c53.c53 c53.A43.a42 A42 a42 f42 3d53.c53.c52 A43.4A43 a43.A43.f52 A43 A44 4A43 g53.f53.d52 A43.4A43 A42 a43 g42 a42 A42.^A42 23a33 a33.A34 A33 a32 f33.4f43 d53 c54 c53.c53 c53 A44 A43.A43 a43.A43.a43.f44 f43.4f43 d53 c54 c53.c53 c53.A43.A43 a43.A43.f52 A43.4A43 g53.f53.d53 A42 3A43 A42 a33 g32 a32 A33 d44 d34 g34 c44 d34 g34 A34 g34 d34 g34 A33 c44 c34 g34 A34 d44 d34 g34 c44 d34 g34 A34 g34 d34 g34 A34 c44 d34 g33 A32 ^A33 ^A32.^A31.";

	outp(0x43, 0xB6); // channel 2, operation 4, mode 3, format 0

	SetTonality(TONALITY);

	printf("Press:\n'0' - Change options\n'1' - Play sound\n'2' - Write Channels Kd\n'3' - Write status words\n'4' - Play Funeral March\n'5' - ImperialMarch\n'6' - CrazyTrain\n'7' - MortalKombat\n'8' - Unravel\n'Esc' - quit\n\n");

	while(c != 27)
	{
		c = getch();
		switch(c)
		{
			case '0':
				printf("Enter new speed in ms:\n");
				scanf("%d", &SPEED);
				printf("Enter new max tonality:\n");
				scanf("%d", &MAX_TONALITY);
				break;
			case '1':
				EnableSound();
				delay(SOUND_DURATION_MILISECONDS);
				DisableSound();
				break;
			case '2':
				ChannelsKd();
				break;
			case '3':
				ReadStatusWords();
				break;
			case '4':
				PlaySong(FuneralMarch);
				break;
			case '5':
				PlaySong(ImperialMarch);
				break;
			case '6':
				PlaySong(CrazyTrain);
				break;
			case '7':
				PlaySong(MortalKombat);
				break;
			case '8':
				PlaySong(Unravel_1);
				PlaySong(Unravel_2);
				break;
			case 27: break;
		}
	}

	clrscr();
	return;
}

//void EnableSound()
//{
//	char port61;
//
//	port61 = inp(0x61);
//	port61 = port61 | 3;
//	outp(0x61, port61);
//}

void EnableSound()
{
	outp(0x61, inp(0x61) | 3);
}

void DisableSound()
{
	char port61;
	port61 = port61 & 0xFFFC;
	outp(0x61, port61);
}

void SetTonality (int frequency)
{
	short value = HerzToShort(frequency);

    outp(0x42, (char)value);		// low byte
    outp(0x42, (char)(value >> 8)); // high byte
}

short HerzToShort(int value)
{
	return 1193180/value;
}

void ReadStatusWords()
{
	unsigned char temp;
	char *str;
	int i;
	str = (char*)calloc(9, sizeof(char));

	for (i = 0; i < 3; i++)
	{
		outp(0x43, 0xe2 + (i * (i + 1)));
		temp = inp(0x40 + i);
		itoa(temp, str, 2);
		printf("Channel state word 0x4%d: %s\n",i, str);
	}

	free(str);
}

void ChannelsKd()
{
	long kd_low, kd_high, kd, kd_max;
	long temp;
	char* str;
	int i;
	long base = 1193180;

	for (i = 0; i < 3; i++) {
		kd_max = 0;

		outp(0x43, 0xD2 + (i * (i + 1)));
		kd_low = inp(0x40 + i);
		kd_high = inp(0x40 + i);
		//printf("%d) kd_low = %d, kd_high = %d\n", i, kd_low, kd_high);
		//kd = kd_high * 256 + kd_low;
		//if (kd_max < kd) {
		//kd_max = kd;
		//}
		kd_max = kd_high * 256 + kd_low;
		temp = base / kd_max;
		//printf("%d | kd_max = %d, kd = %d\n", temp, kd_max, kd);
		printf("Channel 0x4%d kd : %X\n", i, base / kd_max);
	}

	free(str);
}

void SingNote(int note, int londnes)
{
	EnableSound();
	SetTonality(note);
	delay(londnes);
	DisableSound();
}

void PlaySong(const char* tabs)
{
	int i = 0;
	char* temp = (char*)malloc(4);
	char note;
	int INote;
	int LNote;
	int oct;
	int time;

	while (tabs[i])
	{
		temp[0] = tabs[i];
		temp[1] = tabs[i + 1];
		temp[2] = tabs[i + 2];
		temp[3] = tabs[i + 3];
		note = tabs[i];

		INote = 0;
		LNote = 0;
		oct = (int)(temp[1] - '0');
		time = (int)(temp[2] - '0');
		switch (note)
		{
		case 'a':
			INote = 28160;
			break;
		case 'A':
			INote = 29833;
			break;
		case 'b':
			INote = 31608;
			break;
		case 'c':
			INote = 16744;
			break;
		case 'C':
			INote = 17739;
			break;
		case 'd':
			INote = 18793;
			break;
		case 'D':
			INote = 19912;
			break;
		case 'e':
			INote = 21096;
			break;
		case 'f':
			INote = 22350;
			break;
		case 'F':
			INote = 23680;
			break;
		case 'g':
			INote = 25088;
			break;
		case 'G':
			INote = 26579;
			break;
		default:
			delay(SPEED / pow(2, (int)(note - '0')));
			i++;
			continue;
			//break;
		}

		INote = INote / pow(2, MAX_TONALITY - oct);
		LNote = SPEED / pow(2, time);
		if (temp[3] == '.')
			LNote = LNote * 1.5;
		SingNote(INote, LNote);
		if (tabs[i + 4] != '^')
			delay(20);
		i += 4;
	}

}

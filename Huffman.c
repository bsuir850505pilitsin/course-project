#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#define SIZE 4096
#define LEFT 1
#define RIGHT 0
char** dictCreate(char *data,unsigned num);
char* Huffman(char *data, unsigned size);
int firstRun(unsigned *msNum, const char *data, unsigned size);
void filter(unsigned *temp, char *msChar, unsigned *msNum);
void shellSort(unsigned a[], char c[], long size);
char* decompress(char *data, unsigned size);
int size;
int main(){
int i;
char c;

char *data, *temp;
size = SIZE;

size = strlen("1111111111111111111111111111111111111112222222222222222222222222222222222222222222222222222222111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111");
//size = 256;
data = (char*)calloc(size, sizeof(char));
//for(i = 0; i < size; i++) data[i] = (char)rand()%256;
strcpy(data, "1111111111111111111111111111111111111112222222222222222222222222222222222222222222222222222222111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111");

	for(i = 0; i < size; i++){
		printf("%c", data[i]);
	}
	puts("");
	printf("%d", size);
	puts("");
	temp = Huffman(data, size);
//	free(data);
	data = decompress(temp, size);
return 0;
}
///декомпрессия данных (буфер со сжатыми данными, размер выходного буфера)
char* decompress(char *data, unsigned size){
	int i;
	char *output;  //выходной буфер
	unsigned input_size = *(short*)(data); //общий размер входного буфера
	unsigned num = (unsigned char)data[2]; //размер словаря
	char *dict = data + 2 + 1; //указатель на словарь
	//char **dictionary;
	unsigned compressed_size = *(short*)(data+2 + 1 + num);//размер сжатых данных
	unsigned bit_offset = (unsigned)(data[2 + 1 + num + 2]); //пустое смещение последнего байта
	char *compressed_data = data + 2 + 1 + num + 2 + 1; //указатель на сжатые данные
	int denominator = 128;
	int ibyte = 0, obyte = 0, index, temp_denom = denominator, bit = 8, value = 0;

	printf("\n\t==============DECOMPRESSING==============\n");
	printf(" %u | %u | ", input_size, num);
	for(i = 0; i < num; i++) printf("%c", dict[i]);
	printf(" | %u | %u | ", compressed_size, bit_offset);
	for(i = 0; i < compressed_size; i++) printf("0x%x ", (unsigned char)compressed_data[i]);

	if(num == 256) denominator = 128;
	else while(1){
		if(num&denominator) break;
		else denominator>>=1;
	}

	output = (char*)calloc(size, sizeof(char));

	printf("\n");
	for(obyte = 0; obyte < size; obyte++){ 
		temp_denom = denominator;
		value = temp_denom;
			do{
				temp_denom >>= 1;
				//printf("%d", (compressed_data[ibyte] & 128)?1:0);
				if(compressed_data[ibyte] & 128){
					value -= temp_denom;
					if(temp_denom == 0){
						value -= 1;
					}
				}
				else {
					//temp_denom >>= 1;
					 value += temp_denom;
				}
				
				if(bit != 1) compressed_data[ibyte] <<= 1;
				bit--;
				if(bit == 0){
					//printf("<8>");
					bit = 8;
					ibyte++;	
				}
				if(value == num) break;
			}while(temp_denom!=0);

			if(value >= num) value = num - 1;
			output[obyte] = dict[value];
		//	printf("<%c-%d>", output[obyte],value);
			printf(" ", output[obyte],value);
		}
	//output = (char*)calloc( 2 + 1 + num + 2 + 1 + obyte, sizeof(char));
	printf("\n");
	for(i = 0; i < size; i++){
		printf("%c", output[i]);
	}
	//printf("\n");
	//printf("this is testing string for course project named as virtual hard disk drive for operation system windows 7 x86\n");
	return output;
}
///созание словаря: символ и его "адрес" 
char** dictCreate(char *data,unsigned num){
	char **dict;
	int i;
	int denominator = 128;
	if(!num) return;
	if(num == 256) denominator = 128;
	else while(1){
		if(num&denominator) break;
		else denominator>>=1;
	}
	dict = (char**)calloc(num, sizeof(char*));
	for(i = 0; i < num; i++){
		dict[i] = (char*)calloc(10, sizeof(char*)); //'значение', количество адресующих бит, биты[0-7]
		dict[i][0] = data[i];
	}
	for(i = 0; i < num; i++){
		int j = 2;
		int temp = denominator;
		int value = temp;
		char flag;
			for(; j < 10; j++){
			if(!temp || value == num) break;
			if(i < value){
				dict[i][1]++;
				dict[i][j] = '1';
				temp >>= 1;
				value -= temp;
			}
			else if( i >= temp){
				dict[i][1]++;
				dict[i][j] = '0';
				temp >>= 1;
				value += temp;
			}
		}
	}
	for(i = 0; i < num; i++){
		int j = 0;
		printf("\n%d - '%c' - %d - ",i,  (unsigned char)dict[i][0], dict[i][0], (unsigned char)dict[i][1]);
		for(; j < (int)dict[i][1]; j++){
			printf("%c ", dict[i][j+2]);
		} 
	}

return dict;
}
///определение индекса символа в словаре
int getCharDictionaryIndex(char **dict, int num, char data){
	int i = 0;
	for(;i < num; i++){
		if(dict[i][0] == data) return i;
		else continue;
	}
}
///осуществляет сжатие входного потока в выходном
char* compress(char *input, unsigned size, char **dictionary, unsigned num){
	int ibyte = 0, obyte = 0, index, ci = 0, bit = 0; 
	int j;
	char *output;
	//input - входной массив
	//output - выходной буфер
	//size - размер входного и выходного массивов
	//dictionary - словарь
	//num - размер словаря (кол-во символов в словаре)
	//obyte - индекс модифицируемого байта выходного буфера
	//bit - индекс бита модифицируемого байта выходного буфера
	//ibyte - индекс сжимаемого байта во входном буфере
	//index - адрес сжимаемого символа из словаря
	printf("\n");
	for(ibyte = 0; ibyte < size; ibyte++){ 
		index = getCharDictionaryIndex(dictionary, num, input[ibyte]);
		//printf("<%c>",input[ibyte]);
		for(ci = 2; ci < (2 + dictionary[index][1]); ci++){
			input[obyte] = input[obyte]|(dictionary[index][ci] - 48);
			//printf("%d", input[obyte]&1);
			if(bit != 7) input[obyte] <<= 1;
			bit++;
			if(bit == 8){
			//printf("<8>");
				bit = 0;
				obyte++;
				input[obyte]=0;
			}
		}
		/*printf(" - '%c' - %u - ", dictionary[index][0], dictionary[index][1]);
		for(j = 0; j < (int)dictionary[index][1]; j++){
			printf("%c", dictionary[index][j+2]);
		}
		printf("\n");*/
	}
	if(bit != 0){
		input[obyte] <<= 7 - bit;
		obyte++;
	}
	output = (char*)calloc( 2 + 1 + num + 2 + 1 + obyte, sizeof(char));
	//где (по порядку): 2 - общий размер буфера
	//1 - размер словаря
	//num - словарь
	//2 - размер сжатых данных
	//1 - "пустое" смещение в последнем байте
	//obyte - сжатые данные
	{
		short *temp;
		temp = (short*)(output);
		*temp = (short)(2 + 1 + num + 2 + 1 + obyte);
	}
	output[2] = num;
	for(ibyte = 0; ibyte < num; ibyte++){
		output[2 + 1 + ibyte] = dictionary[ibyte][0];
	}
	//printf("\nsizeof(short) = %d", sizeof(short));
	{
		short *temp;
		temp = (short*)(output + 2 + 1 + num);
		*temp = (short)obyte;
	}
	output[2 + 1 + num + 2] = bit;
	for(ibyte = 0; ibyte < obyte; ibyte++){
		output[2 + 1 + num + 2 + 1 + ibyte] = input[ibyte];
	}
	printf("\n\t==============COMPRESSING==============\n");
	printf(" %u | %u | ", *(short*)(output), (unsigned)output[2]);
	for(ci = 0; ci < (unsigned char)output[2]; ci++)	printf("%c", output[ci + 3]);
	printf(" | %u | %u | ", *(short*)(output+(unsigned)output[2] + 2 + 1), (unsigned)output[(unsigned)output[2] + 2 + 1 + 2]);
	for(ci = 0; ci < *(short*)(output+(unsigned)output[2] + 2 + 1); ci++) printf("0x%x ", (unsigned char)output[(unsigned)output[2] + 2 + 1 + 2 + 1 + ci]);

	return output;
}
///данная функция производит сжатие алгоритмом Хаффмана
char* Huffman(char *data, unsigned size){
	char		*msChar;
	char		**dictionary;
	unsigned	*msNum, *temp;
	unsigned	num, i, j;
	struct root	*base;

	//первое прохождение по алгоритму Хаффмана
	temp = (unsigned*)calloc(256, sizeof(unsigned));

	num = firstRun(temp, data, size);
	
	msChar = (char*)calloc(num, sizeof(char));
	msNum = (unsigned*)calloc(num, sizeof(unsigned));

	filter(temp,msChar,msNum);

	free((void*)temp);
	
	shellSort(msNum, msChar, num);

	//for(i = 0; i < num; i++) printf(" '%c' ", (unsigned char)msChar[i]);
	//for(i = 0; i < num; i++) printf("'%c' - %d\n", (unsigned char)msChar[i], (unsigned int)msNum[i]);
	
	free((void*)msNum);

	//создание словаря
	dictionary = dictCreate(msChar, num);
	
	free((void*)msChar);
	
	//sChar = (char*)calloc(size, sizeof(char)); 
	
	//compress(data, msChar, size, dictionary, num);
	return compress(data, size, dictionary, num);
}
///Первый проход - считаем повторение символов
int firstRun(unsigned *msNum, const char *data, unsigned size){
	unsigned i,num;
	unsigned char j;
	for(i = 0, j = 0, num = 0; i < size; i++){
		j = (unsigned)data[i];
		if(msNum[j]++ == 0) num++;
	}
return num;
}
///данная функция отсеивает символы которые не встречались ни разу
void filter(unsigned *temp, char *msChar, unsigned *msNum){
	int i;
	int j = 0;
	for(i = 0, j = 0; i < 256; i++){
		if(temp[i] == 0) continue;
		else {
			msChar[j] = (char)i;
			msNum[j] = temp[i];
			j++;
		}
	}
}
///рабочая функция алгоритма сортировки
int increment(long inc[], long size) {
  int p1, p2, p3, s;

  p1 = p2 = p3 = 1;
  s = -1;
  do {
    if (++s % 2) {
      inc[s] = 8*p1 - 6*p2 + 1;
    } else {
      inc[s] = 9*p1 - 9*p3 + 1;
      p2 *= 2;
      p3 *= 2;
    }
	p1 *= 2;
  } while(3*inc[s] < size);  

  return s > 0 ? --s : 0;
}
///сортировка Шелл (по возрастанию)
void shellSort(unsigned msNum[], char msChar[], long size) {
  long inc, i, j, seq[40];
  int s;

  // вычисление последовательности приращений
  s = increment(seq, size);
  while (s >= 0) {
	// сортировка вставками с инкрементами inc[] 
	inc = seq[s--];
    for (i = inc; i < size; i++) {
      unsigned temp = msNum[i];
	  char tempc = msChar[i];
      for (j = i-inc; (j >= 0) && (msNum[j] > temp); j -= inc){
        msNum[j+inc] = msNum[j];
		msChar[j+inc] = msChar[j];
	  }
		msNum[j+inc] = temp;
		msChar[j+inc] = tempc;
    }
  }
}
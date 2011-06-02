#include "compress.h"

void* compress(void* data, unsigned int size){
	struct character* values;
	struct tree* root;
	struct dictionary **dict;
	char *c_data;
	int i;
	unsigned v_size = 0;

	values = pass0(data, size, &v_size);

	root = createTree(values, v_size);

	dict = createDictionary(root, values, v_size);
	
	c_data = pass1(data, size, dict, v_size);
	
	printPackageInfoAndData(getPackageData((void*)c_data), (void*)c_data);

	myFree((void*) values);
	myFree((void*) root);
	for(i = 0; i < v_size; i++){
		myFree((void*)dict[i]);
	}
	myFree((void*)dict);
return (void*)c_data; 
}

void* decompress(void* data){
	struct character* values;
	struct tree* root;
	struct tree* temp;
	struct packageData inf;
	char* d_data;
	char* c_data;
	unsigned int v_size;
	unsigned int n_bit;
	unsigned int i_index, o_index;
	int i;
	c_data = (char*)data;
	inf = getPackageData((void*)c_data);
	d_data = (char*)myMalloc(inf.decompressedSize * sizeof(char));
	
	v_size = inf.numOfSymbolsInDictionary;
	//создание массива с данными о кодированных символах и их количестве в кодированном массиве
	values = (struct character*)myMalloc(v_size * sizeof(struct character));
	for(i = 0; i < v_size; i++){
		values[i].value = c_data[inf.dictionaryOffset + i*(sizeof(unsigned short)+sizeof(char))]; 
		values[i].num = *(unsigned short*)(c_data + inf.dictionaryOffset + i*(sizeof(unsigned short)+ sizeof(char))  + sizeof(char));
	}
	//создание дерева
	root = createTree(values, v_size);
	//декомпрессия данных
	for(i_index = 0, o_index = 0, temp = root; i_index < inf.compressedSize; i_index++){
		for(n_bit = 0; n_bit < 8; n_bit++){
			if(temp->left != NULL && temp->right != NULL){
				if(((c_data[i_index + inf.dataOffset] << n_bit)&128 ? RIGHT : LEFT) == RIGHT) {
					temp = temp->right;
				}
				else {  
					temp = temp->left; 
				}
			}
			if(temp->left == NULL && temp->right == NULL){
					d_data[o_index] = temp->ch->value;
					o_index++;
					temp = root;
					if(i_index == (inf.compressedSize - 1) &&  (8 - n_bit) == inf.nullBitsInLastByte) break;
			}
				
		}
	}
	myFree((void*)root);
	myFree((void*)values);

	return (void*)d_data;
}

void* myMalloc(int size){
return calloc(size, 1);
}

void myFree(void *mem){
	free(mem);
}

struct packageData getPackageData(void* input){
	char *data;
	struct packageData inf;
	data = (char*)input;

	inf.compressedSize = *(unsigned short*)(data);
	inf.decompressedSize = *(unsigned short*)(data + sizeof(unsigned short)); 
	inf.numOfSymbolsInDictionary = data[2*sizeof(unsigned short)]; 
	inf.nullBitsInLastByte = data[2*sizeof(unsigned short) + sizeof(char)];
	inf.dictionaryOffset = 2*sizeof(unsigned short) + 2*sizeof(char);
	inf.dataOffset = 2*sizeof(unsigned short) + 2*sizeof(char) + inf.numOfSymbolsInDictionary*(sizeof(unsigned short) + sizeof(char));
return inf;
}

struct character* pass0(void* data, unsigned int size, unsigned int *v_size){
	struct character* temp;
	struct character *values;
	int i, n;
	char* dat = (char*)data;
	values = (struct character*)myMalloc(256 * sizeof(struct character));
	for(i = 0; i < 256; i++) { values[i].value = (char)i; values[i].num = 0; }
	//считаем количество повторений каждого символа
	for(i = 0; i < size; i++) {
		n = (unsigned char)dat[i];
		values[n].num++;
	}
	//считаем количество повторяющихся символов
	for(i = 0, n = 0; i < 256; i++) {
		if(values[i].num) n++;
	}
	//сортировка по убыванию
	temp = (struct character*)myMalloc(n * sizeof(struct character));
	*v_size = n;
	for(i = 0, n = 0; i < 256; i++){
		int j = 0;
		if(values[i].num == 0) continue;
		else{
			int k;
			while(values[i].num <= temp[j].num){
				if(j < *v_size - 1) j++;
				else break;
			}
			if(temp[j].num != 0 && temp[j].value != 0){
				k = n;
				while(k > j){	
					temp[k] = temp[k - 1];
					k--;
				}
			}
			temp[j].value = values[i].value;
			temp[j].num = values[i].num;
			n++;
		}
	}
	myFree((void*)values);
return temp;
}

struct tree* createTree(struct character* values, unsigned int v_size){
	struct tree* root;
	struct tree** list;
	unsigned int index, l_size;
	list = (struct tree**)myMalloc(v_size*sizeof(struct tree*));
	l_size = v_size;

	for(index = 0; index < v_size; index++){
		list[index] = (struct tree*)myMalloc(1*sizeof(struct tree));
		list[index]->ch = values + index;
	}
	while(l_size != 1){
			int min1, min2;
			for(index = 1, min1 = 0; index < v_size; index++){
				if(list[index] == NULL) continue;
				if(list[index]->ch->num <= list[min1]->ch->num){
					min1 = index;
				}
			}
			for(index = 1, min2 = 0; index < v_size; index++){
				if(index == min1 || list[index] == NULL) continue;
				if(!min1 && !min2){
					min2 = index;
				}
				else {
					if(list[index]->ch->num <= list[min2]->ch->num){
					min2 = index;
					}
				}
			}
			if(min1 != min2){
				struct tree* temp;
				temp = (struct tree*)myMalloc(1*sizeof(struct tree));
				temp->right = list[min1];
				temp->left = list[min2];
				temp->ch = (struct character*)myMalloc(1*sizeof(struct tree));
				temp->ch->num = temp->left->ch->num + temp->right->ch->num;
				if(min2 < min1){
					list[min2] = temp;
					list[min1] = NULL;
				}
				else {
					list[min1] = temp;
					list[min2] = NULL;
				}
				l_size--;
			}
	}
	if(l_size == 1){
		if(list[0]->left) list[0]->ch->num += list[0]->left->ch->num;
		if(list[0]->right) list[0]->ch->num += list[0]->right->ch->num;
	}
	root = list[0];
	myFree((void*)list);
return root;
}

int searchCharInTree(struct tree* root, char data, struct code* code){
	if(root->right)	if(searchCharInTree(root->right, data, code)){
			code->bit_code >>= 1;
			code->bit_code = code->bit_code|RIGHT*128;
			code->n_bit++;
			return 1;
	}
	if(root->left) if(searchCharInTree(root->left, data, code)){
			code->bit_code >>= 1;
			code->bit_code = code->bit_code|LEFT*128;
			code->n_bit++;
			return 1;
	}
	if(root->left == NULL && root->right == NULL) if(root->ch->value == data){ return 1;}
return 0;
}

struct dictionary** createDictionary(struct tree* root, struct character* values, unsigned int v_size){
	struct dictionary** dict;
	unsigned int index;
	dict = (struct dictionary**)myMalloc(v_size*sizeof(struct dictionary*));
	for(index = 0; index < v_size; index++){
		int i;
		dict[index] = (struct dictionary*)myMalloc(1*sizeof(struct dictionary));
		dict[index]->ch = values + index;
		dict[index]->cd = (struct code*)myMalloc(1*sizeof(struct code));
		searchCharInTree(root, values[index].value, dict[index]->cd);
	}
return dict;
}

int getCharIndex(char data, struct dictionary** dict, unsigned int v_size){
	int i;
	for(i = 0; i < v_size; i++){
		if( dict[i]->ch->value == data ) return i;
	}
return v_size + 1;
}

char* pass1(void* i_data, unsigned int size, struct dictionary** dict, unsigned int v_size){
	char *c_data;
	char *data;
	unsigned int i_index, o_index, index_bit, code_bit, outbyte_bit, char_index;

	data = (char*)i_data;
	c_data = (char*)myMalloc(size*sizeof(char));
	
	for(i_index = 0, o_index = 0, outbyte_bit = 0; i_index < size; i_index++){
		char_index = getCharIndex(data[i_index] , dict,  v_size);
		for(code_bit = 0; code_bit < dict[char_index]->cd->n_bit; code_bit++ ){
				c_data[o_index] = c_data[o_index] | (((dict[char_index]->cd->bit_code << code_bit ) & 128)? RIGHT : LEFT );
				if(outbyte_bit != 7){
					c_data[o_index] <<= 1;
					outbyte_bit++;
				}
				else{
					outbyte_bit = 0;
					o_index++;
				}
		}
	}
	if(outbyte_bit != 0){
		c_data[o_index] <<= 7 - outbyte_bit;
		o_index++;
	}
	//создание пакета данных
	{
		char *temp;
		int dict_offset, data_offset, dict_size, full_size;
		unsigned short *c_size, *d_size;

		dict_offset = sizeof(unsigned short) + sizeof(unsigned short) + sizeof(char) + sizeof(char);
		dict_size = v_size * (sizeof(char) + sizeof(unsigned short));
		data_offset = dict_offset + dict_size;
		full_size = data_offset + o_index;
		temp = (char*)myMalloc(full_size);

		
		c_size = (unsigned short*)temp;
		d_size = (unsigned short*)(temp + sizeof(unsigned short));
		*c_size = o_index;	//размер сжатых данных
		*d_size = size;		//исходный размер
		temp[sizeof(unsigned short) + sizeof(unsigned short)] = (char)v_size; //количество элементов в словаре
		temp[sizeof(unsigned short) + sizeof(unsigned short) + sizeof(char)] = (char)outbyte_bit; //количество пустых бит в крайнем байте

		for(i_index = 0; i_index < v_size; i_index++){
			temp[dict_offset + i_index*(sizeof(unsigned short) + sizeof(char))] = dict[i_index]->ch->value;
			c_size = (unsigned short*)(temp + dict_offset + i_index*(sizeof(unsigned short) + sizeof(char)) + sizeof(char));
			*c_size = dict[i_index]->ch->num;
		}
		for(i_index = 0; i_index < o_index; i_index++){
			temp[data_offset + i_index] = c_data[i_index];
		}
		myFree((void*)c_data);
		c_data = temp;
	}

return c_data;
}


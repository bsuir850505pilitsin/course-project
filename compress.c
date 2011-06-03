#include "compress.h"
#ifdef ALLOC_PRAGMA_1
#pragma alloc_text(PAGE, compress)
#pragma alloc_text(PAGE, decompress)
#pragma alloc_text(PAGE, myMalloc)
#pragma alloc_text(PAGE, myFree)
#pragma alloc_text(PAGE, getPackageData)
#pragma alloc_text(PAGE, pass0)
#pragma alloc_text(PAGE, createTree)
#pragma alloc_text(PAGE, searchCharInTree)
#pragma alloc_text(PAGE, createDictionary)
#pragma alloc_text(PAGE, getCharIndex)
#pragma alloc_text(PAGE, pass1)
#endif
PVOID compress(PVOID data, USHORT size){
	struct character* values;
	struct tree* root;
	struct dictionary **dict;
	PUCHAR c_data;
	USHORT i;
	USHORT v_size = 0;
	PAGED_CODE();

	values = pass0(data, size, &v_size);
	root = createTree(values, v_size);
	dict = createDictionary(root, values, v_size);
	c_data = pass1(data, size, dict, v_size);
	getPackageData((PVOID)c_data);
	for(i = 0; i < v_size; i++){
		myFree((PVOID)(dict[i]));
	}
	myFree((PVOID)dict);
	myFree((PVOID) values);
	myFree((PVOID) root);
	myFree(data);
return (PVOID)c_data; 
}

PVOID decompress(const PVOID data){
	struct character* values;
	struct tree* root;
	struct tree* temp;
	struct packageData inf;
	PUCHAR d_data;
	PUCHAR c_data;
	USHORT v_size;
	USHORT n_bit;
	USHORT i_index, o_index;
	USHORT i;
	PAGED_CODE();

	c_data = (PUCHAR)data;
	inf = getPackageData((PVOID)c_data);
	d_data = (PUCHAR)myMalloc(inf.decompressedSize * sizeof(UCHAR));
	DbgPrint("<mark%d>", 1);
	v_size = inf.numOfSymbolsInDictionary;
	//создание массива с данными о кодированных символах и их количестве в кодированном массиве
	values = (struct character*)myMalloc(v_size * sizeof(struct character));
	DbgPrint("<mark%d>", 2);
	for(i = 0; i < v_size; i++){
		values[i].value = c_data[inf.dictionaryOffset + i*(sizeof(USHORT)+sizeof(UCHAR))]; 
		values[i].num = *(PUSHORT)(c_data + inf.dictionaryOffset + i*(sizeof(USHORT)+ sizeof(UCHAR))  + sizeof(UCHAR));
	}
	DbgPrint("<mark%d>", 3);
	if(v_size == 1){
		for(i = 0; i < inf.decompressedSize; i++){
			d_data[i] = values[0].value;
		}
		myFree((PVOID)values);
		return (PVOID)d_data;
	}
	DbgPrint("<mark%d>", 4);
	//создание дерева
	root = createTree(values, v_size);
	//декомпрессия данных
	for(i_index = 0, o_index = 0, temp = root; i_index <= inf.compressedSize; i_index++){
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
	DbgPrint("<mark%d>", 5);
	myFree((PVOID)root);
	myFree((PVOID)values);
	//myFree(data);
	return (PVOID)d_data;
}

PVOID myMalloc(int size){
	PVOID temp;
	PAGED_CODE();

		temp = ExAllocatePool( NonPagedPool, size);
		if(temp == NULL) DbgPrint("ERROR ALLOCATING POOL FOR (DE)COMPRESSING");
		RtlZeroMemory(temp, size);
return temp;
}

void myFree(void *mem){
	PAGED_CODE();

	ExFreePool(mem);
}

struct packageData getPackageData(PVOID input){
	PUCHAR data;
	struct packageData inf;
	PAGED_CODE();

	data = (PUCHAR)input;

	inf.compressedSize = *(PUSHORT)(data);
	inf.decompressedSize = *(PUSHORT)(data + sizeof(USHORT)); 
	inf.numOfSymbolsInDictionary = (UCHAR)data[2*sizeof(USHORT)]; 
	inf.nullBitsInLastByte = (UCHAR)data[2*sizeof(USHORT) + sizeof(UCHAR)];
	inf.dictionaryOffset = (USHORT)(2*sizeof(USHORT) + 2*sizeof(UCHAR));
	inf.dataOffset = (USHORT)(inf.dictionaryOffset + (inf.numOfSymbolsInDictionary+1)*(sizeof(USHORT) + sizeof(UCHAR)));
	DbgPrint("%d - %d - %d - %d - %d - %d", (USHORT)inf.compressedSize, (USHORT)inf.decompressedSize, (UCHAR)inf.numOfSymbolsInDictionary, (UCHAR)inf.nullBitsInLastByte, (USHORT)inf.dictionaryOffset, (USHORT)inf.dataOffset);
return inf;
}

struct character* pass0(PVOID data, USHORT size, PUSHORT v_size){
	struct character* temp;
	struct character *values;
	USHORT i, n;
	PUCHAR dat = (PUCHAR)data;
	PAGED_CODE();

	values = (struct character*)myMalloc(256 * sizeof(struct character));
	for(i = 0; i < 256; i++) { values[i].value = (UCHAR)i; values[i].num = 0; }
	//считаем количество повторений каждого символа
	for(i = 0; i < size; i++) {
		n = (UCHAR)dat[i];
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
		USHORT j = 0;
		if(values[i].num == 0) continue;
		else{
			USHORT k;
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
	myFree((PVOID)values);
return temp;
}

struct tree* createTree(struct character* values, USHORT v_size){
	struct tree* root;
	struct tree** list;
	USHORT index, l_size;
	PAGED_CODE();

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
	myFree((PVOID)list);
return root;
}

int searchCharInTree(struct tree* root, UCHAR data, struct code* code){
	PAGED_CODE();

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

struct dictionary** createDictionary(struct tree* root, struct character* values, USHORT v_size){
	struct dictionary** dict;
	USHORT index;
	PAGED_CODE();

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

USHORT getCharIndex(UCHAR data, struct dictionary** dict, USHORT v_size){
	USHORT i;
	PAGED_CODE();

	for(i = 0; i < v_size; i++){
		if( dict[i]->ch->value == data ) return i;
	}
return v_size + 1;
}

PUCHAR pass1(PVOID i_data, USHORT size, struct dictionary** dict, USHORT v_size){
	PUCHAR c_data;
	PUCHAR data;
	USHORT i_index, o_index, index_bit, code_bit, outbyte_bit, UCHAR_index;
	PAGED_CODE();

	data = (PUCHAR)i_data;
	c_data = (PUCHAR)myMalloc(size*sizeof(UCHAR));
	
	for(i_index = 0, o_index = 0, outbyte_bit = 0; i_index < size; i_index++){
		UCHAR_index = getCharIndex(data[i_index] , dict,  v_size);
		for(code_bit = 0; code_bit < dict[UCHAR_index]->cd->n_bit; code_bit++ ){
				c_data[o_index] = c_data[o_index] | (((dict[UCHAR_index]->cd->bit_code << code_bit ) & 128)? RIGHT : LEFT );
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
		PUCHAR temp;
		unsigned dict_offset, data_offset, dict_size, full_size;
		PUSHORT c_size, d_size;

		dict_offset = sizeof(USHORT) + sizeof(USHORT) + sizeof(UCHAR) + sizeof(UCHAR);
		dict_size = v_size * (sizeof(UCHAR) + sizeof(USHORT));
		data_offset = dict_offset + dict_size;
		full_size = data_offset + o_index;
		temp = (PUCHAR)myMalloc(full_size);
		
		c_size = (PUSHORT)temp;
		d_size = (PUSHORT)(temp + sizeof(USHORT));
		*c_size = (USHORT)o_index;	//размер сжатых данных
		*d_size = (USHORT)size;		//исходный размер
		temp[sizeof(USHORT) + sizeof(USHORT)] = (UCHAR)(v_size - 1); //количество элементов в словаре
		temp[sizeof(USHORT) + sizeof(USHORT) + sizeof(UCHAR)] = (UCHAR)outbyte_bit; //количество пустых бит в крайнем байте
		for(i_index = 0; i_index < v_size; i_index++){
			temp[dict_offset + i_index*(sizeof(USHORT) + sizeof(UCHAR))] = dict[i_index]->ch->value;
			c_size = (PUSHORT)(temp + dict_offset + i_index*(sizeof(USHORT) + sizeof(UCHAR)) + sizeof(UCHAR));
			*c_size = dict[i_index]->ch->num;
		}
		for(i_index = 0; i_index < o_index; i_index++){
			temp[data_offset + i_index] = c_data[i_index];
		}
		
		myFree((PVOID)c_data);
		c_data = temp;
	}

return c_data;
}


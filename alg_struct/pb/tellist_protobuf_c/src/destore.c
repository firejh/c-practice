
#include "pc_public.h"
#include "tellist.pc.h"
#include <stdlib.h>
#include <stdio.h>

#define	PHONE_NUM_TYPE		unsigned long long int

int main (int argc, const char * argv[]) 
{
	unsigned int		idx;
	FILE*				file;
	char*				file_name = "out";
	unsigned int		file_size;
	unsigned int		tel_num;
	PHONE_NUM_TYPE		phone_num;
	phone_contact_uin_tel*	tel;

	file = NULL;
	do {
		file = fopen(file_name, "r");
		if (!file)	{
			fprintf(stderr, "fopen(%s, r) = %p", file_name, file);
			break;
		}

		fseek(file, 0L, SEEK_END);
		file_size = ftell(file);
		if (!file_size)	{
			fprintf(stderr, "ftell(file = %p) = %u", file, file_size);
			break;
		}
		rewind(file);
		char buf[file_size];
		fread(buf, sizeof(char), sizeof(buf), file);

		PC_UNPACK(phone_contact_uin_tel_list, tel_list, buf, file_size);
		if (!tel_list)	{
			break;
		}
		phone_num = 0;
		tel_num = tel_list->n_tel;
		for (idx = 0; idx < tel_num; idx++)	{
			tel = tel_list->tel[idx];
			if (tel->has_num)	{
				phone_num += tel->num;
				printf("idx:{name = %4s, tel = %13llu}\n", tel->name, phone_num);
			}
		}

		PC_UNPACK_FREE(phone_contact_uin_tel_list, tel_list);
	} while(0);

	if (file)	{
		fclose(file);
	}

	return 0;
}

#include <stdio.h>

#include "common.h"

int main(void)
{

	char *beijin = "back.jpeg";
	show_beijin(beijin);

	int ret,i,j;
	ret = sqlite3_open("plate_number.db",&db);
	if(ret)
	{
		perror("sqlite3_open");
		return -1;
	}

	Creat_table(db);
	Creat_evertable(db);

	int fd = open("/dev/input/event0", O_RDONLY);
	if(fd == -1)
	{
		perror("open failed");
		exit(0);
	}

	pthread_t tid;
	pthread_create(&tid, NULL, camera, NULL);

	action = 0;
	action2 = 0;
	jpg_num = 1;
	num_ever = 0;
	lock_ever = 0;
	lock_jpgnum = 0;

	int car_max = 100;

	while(1)
	{
		action = touch_action(fd);
		if(action == 10)
		{
			del_table(db);
			sqlite3_close(db);
			break;
		}

		if(action == 1)
		{
			cardid = read_rfid();
			if(cardid == 0)
				continue;
			insert_info(db, cardid);
			action = 0;
		}
		
		if(action == 2)
		{
			cardid = read_rfid();
			if(cardid == 0)
				continue;
			action2 = 2;
			sleep(2);
			sqlite_find(db, cardid);
			action = 0;
		}
		
		if(action == 4)
		{
			cardid = read_rfid();
			if(cardid == 0)
				continue;
			sqlite_del(db, cardid);
			action = 0;
		}

		//查看车库
		if(action == 3)
		{
			sqlite_action(car_max);
			action = 0;
		}
	}

	return 0;
}

<<<<<<< HEAD
////nihao
=======
////nihao
>>>>>>> featurel

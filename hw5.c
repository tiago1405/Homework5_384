#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <libgen.h>
#include <errno.h>
#include <linux/limits.h>
#include <errno.h>
#include <time.h>

int main (int argc, char* argv[])
{
	bool opt_h, opt_d, opt_m, opt_t;									//Creating variables and initialzing them
	time_t rawtime;
	int saved_errno;
	opt_h = false;	opt_m = false;
	opt_d = false;	opt_t = false;
	char* direc = NULL;

	int opt;
	while((opt = getopt(argc, argv, "hd:mt")) != -1)					// Command line opts
	{
		switch(opt)
		{
			case 'h':	opt_h = true;					break;			// Help Case
			case 'd':	opt_d = true; direc = optarg;	break;			// Customize Backup
			case 'm':	opt_d = true;					break;			// Meta Data Off
			case 't':	opt_t = true; 					break;			// Add time to name
		}
	}
	
	if(opt_h == true)													// Print help info and successfully 
	{																	// terminate after
		printf("Write helpful information to the user \n");
		printf("Write helpful information to the user \n");
		exit(EXIT_SUCCESS);
	}
	
	char* path = "/home/tiago/Documents/dummy";							// Original File path
	char* bpath = "/home/tiago/Documents/";								// Backup Path
	if(opt_d == true)													// Costumize the directory path
	{
		bpath = direc;
	}

	int filed = inotify_init();											// Creating filed descriptor
	int fwatch = inotify_add_watch(filed, path, IN_MODIFY);				// Creting file watcher to monitor changes
	//Check for any errors with the above calls
	int revcount = 0;													// Counter for rev
	while(1)
	{
		ssize_t length = (5120);										// 5kb size
		char buf[length];
		int reader = read(filed, buf, length);
		if(read < 0)
		{
			perror("Error at reader: ");
			return -1;
		}
		char* i;	

		for(i = buf; i < buf + reader;)									// Keep reading
		{
			struct inotify_event* event = (struct inotify_event*)i;		//	Creating event notifier
			if(event < 0)
			{
				perror("Event ERROR: ");
				return -1;
			}
			if((event->mask) == 2)										// If the file was modified
			{
				char fbpath[1000];											// Full backup path

				if(opt_t)												//Add time to file name
				{
					char timebuf[100];
					char fileN[100];
					char* pathd = strdup(path);
					char* basen = basename(pathd);

					struct tm *timestr;
					time(&rawtime);
					timestr = localtime(&rawtime);

					snprintf(timebuf, 100, "%d%d%d%d%d%d",
							(int)((timestr->tm_year)+1900),(timestr->tm_mday),
							(timestr->tm_mon),(timestr->tm_hour),
							(timestr->tm_min), (timestr->tm_sec));
					snprintf(fileN, 100,"%s",basen);
					strcpy(fbpath, bpath);
					strcat(fbpath, fileN);	strcat(fbpath, timebuf);
				}
				else													//Add whatever rev the counter is at
				{
					char fileN[100];
					char* pathd = strdup(path);
					char* basen = basename(pathd);

					printf("%s\n", basen);	
					snprintf(fileN, 100,"%s",basen);
					char rev[100];
					snprintf(rev, 100, "_rev%d", revcount);
					strcpy(fbpath, bpath);								// Copying the backup path to fbpath 
					strcat(fbpath, fileN); strcat(fbpath, rev);			// Concatenating rev to file name in fbpath
				}

				int orgf=0,bacf=0;										// Original and Backup File
				orgf = open(path, O_RDONLY);							// Open the original file as read only
				if(orgf < 0)
				{
					perror("ERROR opening original file: ");
					return -1;
				}
				printf("%s\n", fbpath);
				bacf = open(fbpath, O_WRONLY | O_CREAT | O_EXCL);	// Create and Open the Backup File
				if(bacf < 0)
				{
					perror("ERROR opening backup file: ");
					return -1;
				}

				char* output = buf;
				ssize_t write2f;									// Write to file
				ssize_t readf = read(orgf, buf, length);			// read file to copy
				if(readf <= 0)
				{
					perror("Readf ERROR");
				}
					///*
				while(readf > 0)									// While there is more to read
				{
					write2f = write(bacf, output, reader);
					if (write2f <= 0)
					{
						perror("Write2f ERROR");
						return -1;
					}

				 	if(write2f >= 0)
				 	{
				 		printf("In IF statement\n");
					 	readf -= write2f;								// Subtract what we read from what we wrote
					 	output += write2f;								// Write to the ouput buffer
						printf("Output: %s\n", output);
					}
					if(write2f <= -1)
					{
						perror("Error in writing to file");
						return -1;
					}
				}
				//*/
				if (close(orgf) < 0)
				{
					perror("Error closing Original File");
					return -1;
				}
				else	close(orgf);

				if (close(bacf) < 0)
				{
					perror("Error closing Backup File");
					return -1;
				}
				else	close(bacf);;

				//	*/

				revcount++;
				i += sizeof(struct inotify_event) + event->len;
			}
			//printf("Out of if-changed statement\n");
		}
	}
	return EXIT_SUCCESS;
}
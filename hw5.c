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
#include <time.h>

int main (int argc, char* argv[])
{
	bool opt_h, opt_d, opt_m, opt_t;									//Creating variables and initialzing them
	time_t time;
	//struct tm *loctime;  
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
	int fwatch = inotify_add_watch(filed, path, IN_MODIFY | IN_ACCESS);	// Creting file watcher to monitor changes
	//Check for any errors with the above calls
	int revcount = 0;													// Counter for rev
	while(1)
	{
		ssize_t length = (5120);										// 5kb size
		char buf[length];
		int reader = read(filed, buf, length);
		// Check for read errors
		char* i;

		for(i = buf; i < buf + reader;)									// Keep reading
		{
			struct inotify_event* event = (struct inotify_event*)i;		//	Creating event notifier
			//Check for errors here
			if((event->mask) == 2)										// If the file was modified
			{
				printf("The file was modified\n");
				//	/*
				char fbpath[1000];											// Full backup path

				if(opt_t)												//Add time to file name
				{
					struct tm *timestr;
					char timebuf[100];
					//time = time (NULL);
					timestr = localtime(&time);

					snprintf(timebuf, 100, "%d_%d_%d_%d_%d",
							(timestr->tm_mday),(timestr->tm_mon),
							(timestr->tm_hour),(timestr->tm_min),
							(timestr->tm_sec));
					

				}
				else													//Add whatever rev the counter is at
				{
					printf("In Else statement\n");
					char fileN[100];
					char* pathd;
					char* basen;
					pathd = strdup(path);
					basen = basename(pathd); 
					printf("%s\n", basen);	
					snprintf(fileN, 100,"%s",basen);
					char rev[100];
					snprintf(rev, 100, "_rev%d", revcount);
					strcpy(fbpath, bpath);								// Copying the backup path to fbpath 
					strcat(fbpath, fileN); strcat(fbpath, rev);			// Concatenating rev to file name in fbpath
				}

				printf("File was named\n");
				int orgf=0,bacf=0;										// Original and Backup File
				printf("Opening orgf");
				orgf = open(path, O_RDONLY);							// Open the original file as read only
				//Check Error when opening
				printf("orgf open");
				printf("Opening bacf");
				bacf = open(fbpath, O_WRONLY | O_CREAT | O_EXCL);		// Create and Open the Backup File
				//Check Error when opening
				printf("bacf open");
				char* output = buf;
				printf("%s",(event->name));
				ssize_t write2f;										// Write to file
				ssize_t readf = read(orgf, buf, length);				// read file to copy
				while(readf > 0)										// While there is more to read
				{
				 	write2f = write(bacf, output, reader);
				 	readf -= write2f;									// Subtract what we read from what we wrote
				 	output += write2f;									// Write to the ouput buffer
				}
				close(orgf);
				// Check for errors closing
				close(bacf);
				// Check for errors closing
				//	*/

				revcount++;
				i += sizeof(struct inotify_event) + event->len;
			}

		}
	}

	return EXIT_SUCCESS;
}
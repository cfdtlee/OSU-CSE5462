/*
 *This program reads a text file and print the following statistics on the screen as
 *well to an output file:
 *the size of the file in bytes
 *number of times the search-string specified in the second argument appeared in the file
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int compare(char *str, char *target);
int main( int number_of_args, char* list_of_args[] ) 
{
	static const int STRLEN = 1001; //max length of stream
	char str[STRLEN]; // str store every chunck of stream
	int count = 0; // store how many times target appear in str 
	int size = 0; // store size of the input file
	if ( number_of_args != 4 )
	{
		printf("you are supposed to provide 3 arguments for input file, search string and output file.\n");
		exit(-1);
	}
	FILE *fin = fopen( list_of_args[1], "r" );
	FILE *fout = fopen( list_of_args[3], "w" );
	char *target = list_of_args[2];
	if (!fin) {
		printf("the first file is invalid\n");
		exit(-1);
	}
	if (!fout) {
		printf("the second file is invalid\n");
		exit(-1);
	}
	while(fgets(str, STRLEN, fin)) 
	// when the file pointer reach the end, fgets will return 0, so while loop won't going on
	{
		if(strlen(str) == 1)
		{
			// skip empty lines
			continue;
		}
		int c, len = strlen(target);
		c = compare(str, target);
		count += c;
		if(c == 0 && strlen(str) == STRLEN - 1) 
		{
			// if there is no target found in str and the length of str is equal to STRLEN - 1,
			// it is possible the target may lay at the end of the trunk of stream
			fseek(fin, -len, SEEK_CUR);
		}
	}
	// at this point, file pointer has reached the end of file, 
	// so ftell can return the size of the file
	size = ftell(fin); 
	fprintf(fout, "Size of file is %d\nNumber of matches = %d\n", size, count);
}

// compare return integer to denote how many times target appers in str
int compare(char *str, char *target)
{
	int count = 0, i = 0, j = 0;
	for(i = 0; i < strlen(str); i++)
	{
		for(j = 0; j < strlen(target) && i+j < strlen(str); j++)
		{
			if(str[i+j] != target[j])
			{
				break;
			}
		}
		if(j == strlen(target))
		{
			count++;
		}
	}
	return count;
}
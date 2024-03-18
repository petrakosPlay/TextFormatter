#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 
Handle @ sign is not a very good option. First it does not currently handle the EOF value, but more
importantly it cannot distinguish between @ signs used for e-mails and other stuff. So it should be used only
for PM TXT files in STATMENT.

leftovers:


Handle command line arguments.
If charsPerLine = 0, this is not valid. If -c xxx is given instead of a number, then it is not valid. If no charsPerLine
is given then it is not valid.

Decide global vs local variable declaration.

There is also a small memory leak. Fix it. (It seems that it is fixed while fixing the EOF bug)
I was actually not taking into consideration the last word of the text. So since I do it now, I also take care to free
respective allocated space. I run some tests using valgrind and it shows no leaks.

Refactor the formatting algorithm. Also make into a function and optionally call it using a pointer to the function.

If one paragraph fits in a line then do not run the formatting argorithm. Just print the text as is.

Handle exit function. You should free the resources before exiting when something unexpected happens.


if chars per line is less than the length of the biggest word then there is a problem.
*/


#define MAX_WORDS_PER_PARAGRAPH 1500
#define MAX_WORD_LENGTH 150
#define DEBUG 0


typedef struct wordDetails
{
    char * word;
    int wordLength;
	int paddedSpaces;
} wordDetails;



//Delete and/or rename the following. Should they be global or declared in main. What is the difference.

char wordBuffer[MAX_WORD_LENGTH];
wordDetails wordsBuffer[MAX_WORDS_PER_PARAGRAPH];


int i, j;
int charsPerLine;
const int INFINITY = MAX_WORDS_PER_PARAGRAPH * MAX_WORD_LENGTH;
//const int INFINITY = -1;

/*------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------*/
/* Helper functions */

void perrorExit(char *errorMsg) {
	fprintf(stderr, "%s\n", errorMsg);
	exit(EXIT_FAILURE);
}

/*
void perrorExit2(char *errorMsg) {
	fprintf(stderr, "%s\n", errorMsg);
	exit(EXIT_FAILURE);
}
*/

void showHelpText(void) {
	fprintf(stdout, "\nFormat text so that each line has the same number of characters\n");
	fprintf(stdout, "Example usage: ./tf -c 100   (for 100 characters per line)\n");
	fprintf(stdout, "\n  -c			specify the desired number of characters per line\n");
	fprintf(stdout, "  -h --help		display this help text\n");
	fprintf(stdout, "  --handleAtSign	handles variables between @ signs in PM statment txt files\n\n");
}


/* Get the number of bytes per character of text */
int numberOfBytesInChar(unsigned char val)
{
    if (val < 128)			return 1;
    else if (val < 224)		return 2;
    else if (val < 240)	    return 3;
    else				    return 4;
}

/* Read a paragraph and store all its words in a buffer (i.e. wordsBuffer) */
int readParagraph(FILE *inputFilePtr, int *wordsCount, int handleAtSignIsEnabled)
{
	memset(wordBuffer, 0, MAX_WORD_LENGTH);
	*wordsCount = 0;
	
    for (i=0; i < MAX_WORDS_PER_PARAGRAPH; i++)
    {
        wordsBuffer[i].word = NULL;
        wordsBuffer[i].wordLength = 0;
		wordsBuffer[i].paddedSpaces = 0;
    }

#if DEBUG1	
	while ( (nextChar = fgetc(inputFilePtr)) != EOF)
	{
		fputc(nextChar, stderr);
	}
#endif	

//Break text into single words and store them in a buffer.
	int nextChar, charCount, consecutiveNewLineCharacters, bytesRemain;
	charCount = consecutiveNewLineCharacters = i = j = 0;
    while( isspace(nextChar = fgetc(inputFilePtr)) )	continue;			//Skip all whitespace at the beginning of the text
    	
	while(nextChar != EOF)
    {	
		if (j == MAX_WORDS_PER_PARAGRAPH)	perrorExit("A given paragraph has too many words.\n");
		
//DO I NEED A SPACE AFTER THE LAST WORD???????		
		
		switch(nextChar)
		{
			
			case ' ':
			case '\t':
			case '\v':
				if (i > 0)
				{
					wordBuffer[i++] = ' ';
					wordBuffer[i] = '\0';
					wordsBuffer[j].word = malloc( (i+1) * sizeof(char) );
					strncpy(wordsBuffer[j].word, wordBuffer, i+1);
					wordsBuffer[j++].wordLength = charCount+1;
					charCount = i = 0;
				}
				break;
				
			case '\n' :
				if (i > 0)
				{
					wordBuffer[i++] = ' ';
					wordBuffer[i] = '\0';
					wordsBuffer[j].word = malloc( (i+1) * sizeof(char) );
					strncpy(wordsBuffer[j].word, wordBuffer, i+1);
					wordsBuffer[j++].wordLength = charCount+1;
					charCount = i = 0;
				}

				if (++consecutiveNewLineCharacters > 1) // \n found two consecutive times
				{
					*wordsCount = j;
					return 0;
				}
				break;
				
			case '@':
				if (handleAtSignIsEnabled)
				{
					wordBuffer[i++] = nextChar;
					bytesRemain = numberOfBytesInChar((unsigned char)nextChar) - 1;
					while (bytesRemain)
					{
						nextChar = fgetc(inputFilePtr);
						wordBuffer[i++] = nextChar;
						bytesRemain--;
					}
					charCount++;
					while ( (nextChar = fgetc(inputFilePtr) ) != '@')
					{
						wordBuffer[i++] = nextChar;
						bytesRemain = numberOfBytesInChar((unsigned char)nextChar) - 1;
						while (bytesRemain)
						{
							nextChar = fgetc(inputFilePtr);
							wordBuffer[i++] = nextChar;
							bytesRemain--;
						}
						charCount++;
					}
					wordBuffer[i++] = nextChar;
					bytesRemain = numberOfBytesInChar((unsigned char)nextChar) - 1;
					while (bytesRemain)
					{
						nextChar = fgetc(inputFilePtr);
						wordBuffer[i++] = nextChar;
						bytesRemain--;
					}
					charCount++;
					break;
				}
				
			default:
				wordBuffer[i++] = nextChar;
				bytesRemain = numberOfBytesInChar((unsigned char)nextChar) - 1;
				while (bytesRemain)
				{
					nextChar = fgetc(inputFilePtr);
					wordBuffer[i++] = nextChar;
					bytesRemain--;
				}
				charCount++;
				consecutiveNewLineCharacters = 0;
				break;
		}
				
		nextChar = fgetc(inputFilePtr);
    }
	//store last word before returning. It is mostly stored already but you need to increase the word count and add the space at the end of the word.
	wordBuffer[i++] = ' ';
	wordBuffer[i] = '\0';
	wordsBuffer[j].word = malloc( (i+1) * sizeof(char) );
	strncpy(wordsBuffer[j].word, wordBuffer, i+1);
	wordsBuffer[j++].wordLength = charCount+1;
	*wordsCount = j;
	return 1;		//means EOF is reached
}


int main(int argc, char **argv)
{
	//int i, j, handleAtSignIsEnabled = 0;
	int handleAtSignIsEnabled = 0;
	for (i = 1; i < argc; i++) 													// Skip argv[0] 
    {
        if (strcmp(argv[i], "-c") == 0)                                         //operations file
        {
            if(++i == argc)
			{
				fprintf(stdout, "You must specify the desired number of characters per line.\n");
				showHelpText();
				exit(EXIT_SUCCESS);
			}
			charsPerLine = atoi(argv[i]);
		}
		else if ( (strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0) )
		{
			showHelpText();
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(argv[i], "--handleAtSign") == 0)	handleAtSignIsEnabled = 1;
    }
	
	
    FILE *inputFilePtr, *outputFilePtr;
    inputFilePtr = outputFilePtr = NULL;

    if ((inputFilePtr = fopen("randomText.txt", "r") ) == NULL )
    {
        fprintf(stderr, "Error while opening randomText.txt\n");
        exit(EXIT_FAILURE);
    }

    if ((outputFilePtr = fopen("formattedText.txt", "w") ) == NULL )
    {
        fprintf(stderr, "Error while opening formattedText.txt\n");
        exit(EXIT_FAILURE);
    }

	int wordsCount, EOFisReached;
	
	while (1)
	{
	#if DEBUG1
		fprintf(stderr, "wordsCount = %d\n", j);
		for (i=0; i < j; ++i)
		{
			//fprintf(outputFilePtr, "%s\n", wordsBuffer[i].word);
			fprintf(outputFilePtr, "NEW\n");
			fprintf(outputFilePtr, "%s\n", wordsBuffer[i].word);
		}
	#endif	
		EOFisReached = readParagraph(inputFilePtr, &wordsCount, handleAtSignIsEnabled);
		
		//Calculate the cost/badness for each possible string of words that can fit in one line.
		if (wordsCount == 0) exit(EXIT_SUCCESS);
		
		
		int curLineLength = 0, curLineStart, nextLineStart, currentCost; //suffixBadness=0;
		int **costBuffer, **whitespace, *badness, *lineBreaks;
		badness = lineBreaks = NULL;
		costBuffer = whitespace = NULL;
		costBuffer = malloc(wordsCount * sizeof(int *));
		whitespace = malloc(wordsCount * sizeof(int *));
		for (i=0; i < wordsCount; i++)
		{
			//*(costBuffer + i) = malloc(wordsCount * sizeof(int));
			costBuffer[i] = malloc(wordsCount * sizeof(int));
			whitespace[i] = malloc(wordsCount * sizeof(int));
		}
		badness = malloc( wordsCount * sizeof(int) );
		lineBreaks = malloc( wordsCount * sizeof(int) );
	 
		for (i=0; i < wordsCount; ++i)
		{
			for (j=0; j < wordsCount; ++j)  costBuffer[i][j] = whitespace[i][j] = INFINITY;
			badness[i] = lineBreaks[i] = INFINITY;
		}
		
		for (i=0; i < wordsCount; ++i)
		{
			curLineLength = 0;
			for (j=i; j < wordsCount; ++j)
			{
				curLineLength += wordsBuffer[j].wordLength;
				if (curLineLength-1 <= charsPerLine)      //current string of words fits in one line without the last empty space
				{
					whitespace[i][j] = charsPerLine - curLineLength +1;
					costBuffer[i][j] = whitespace[i][j] * whitespace[i][j];
				}
				else break;
			}
		}
		
	#if DEBUG
		for (i=0; i < wordsCount; ++i)
		{
			for (j=0; j < wordsCount; ++j)
			{
				fprintf(stderr, "%3d,", costBuffer[i][j]);
			}
			fprintf(stderr, "\n");
		}
	#endif

		nextLineStart = wordsCount;
		curLineStart  = wordsCount - 1;
		while (curLineStart >= 0)		//where to start building the line
		{
			while (curLineStart < nextLineStart)
			{
				if (costBuffer[curLineStart][nextLineStart-1] < INFINITY)	//the current set of  words fits in a line 
				{
				//	suffixBadness = (nextLineStart == wordsCount) ? 0 : badness[nextLineStart];	//the best i can do after the specified word
					
					
					if (nextLineStart == wordsCount)	currentCost=0;		//new lines
					//else	currentCost = costBuffer[curLineStart][nextLineStart-1] + suffixBadness;
					else	currentCost = costBuffer[curLineStart][nextLineStart-1] + badness[nextLineStart];
					
					
					if (currentCost < badness[curLineStart])
					{
						badness[curLineStart] = currentCost;
						lineBreaks[curLineStart] = nextLineStart;
					}
					//badness[curLineStart] = costBuffer[curLineStart][nextLineStart-1]
					//lineBreaks[curLineStart] = nextLineStart;
				}
				nextLineStart --;
			}
			nextLineStart = wordsCount;
			curLineStart--;
		}
		
		
		i = 0;
		j = lineBreaks[i];
		int curLineWords = j - i, padDistance = 0, k = 0;			//number of words that will be printed in the current line
		int curLineSpacePositions = curLineWords -1;				//the number of positions between words that can be padded with spaces
		//while (j != INFINITY)
		while (1)
		{
			if (j == wordsCount || j == -1)		//j==-1 suggests there is only one line, but better check it again.
			{
				//fprintf(outputFilePtr, "\n");
				for( ; i < j; ++i)	fprintf(outputFilePtr, "%s", wordsBuffer[i].word);
				break;
			}
			if (whitespace[i][j-1] > 0 && curLineWords > 1)
			{
				while (whitespace[i][j-1] >= curLineSpacePositions)
				{
					for (k = i; k < i+curLineSpacePositions; k++)	wordsBuffer[k].paddedSpaces++;
					whitespace[i][j-1] -= curLineSpacePositions;
				}
				//padSpacesEvenly(i, j);
				if (whitespace[i][j-1] > 0)
				{
					padDistance = curLineSpacePositions / whitespace[i][j-1];
					//for (k = i+padDistance; k < j; k+= padDistance)
					for (k = i+padDistance-1; k < j-1 && whitespace[i][j-1] > 0; k+= padDistance)
					{
						wordsBuffer[k].paddedSpaces++;
						whitespace[i][j-1]--;
					}
				}
			}
			while (i < j)
			{
				//void writeLine()
				if (i == j-1)
				{
					//wordsBuffer[i].word[wordsBuffer[i].wordLength+1] = '\0';
					fprintf(outputFilePtr, "%s", wordsBuffer[i].word);
				}
				else
				{
					fprintf(outputFilePtr, "%s", wordsBuffer[i].word);
					//if (--curLineWords > 1)
					//{
						for (k=0; k < wordsBuffer[i].paddedSpaces; k++)		fprintf(outputFilePtr, " ");
					//}
				}
				++i;
			}
			j = lineBreaks[i];
			/*if (j == wordsCount || j == -1)		//j==-1 suggests there is only one line, but better check it again.
			{
				fprintf(outputFilePtr, "\n");
				for( ; i < j; ++i)	fprintf(outputFilePtr, "%s", wordsBuffer[i].word);
				break;
			}*/
			
			//if (j == INFINITY) break;
			fprintf(outputFilePtr, "\n");
			curLineWords = j - i;
			curLineSpacePositions = curLineWords -1;
		}	
		
		fprintf(outputFilePtr, "\n\n");
		
		
		for (i=0; i < wordsCount; ++i)
		{
			free(wordsBuffer[i].word);
			free(*(costBuffer+i));
			free(*(whitespace+i));
		}
		free(lineBreaks);
		free(badness);
		free(costBuffer);
		free(whitespace);
		
		if (EOFisReached) break;
	}


	fclose(outputFilePtr);
    fclose(inputFilePtr);	
    return EXIT_SUCCESS;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
static int INFINITY = -1;
char wordBuffer[MAX_WORD_LENGTH];
wordDetails wordsBuffer[MAX_WORDS_PER_PARAGRAPH];
int i, j;
int charsPerLine;


/*------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------*/
/* Helper functions */

void perrorExit(char *errorMsg) {
	fprintf(stderr, "%s\n", errorMsg);
	exit(EXIT_FAILURE);
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
	int nextChar, charCount, consecutiveNewLineCharacters, remainingBytes;
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
					return 1;
				}
				break;
				
			case '@':
				if (handleAtSignIsEnabled)
				{
					wordBuffer[i++] = nextChar;
					remainingBytes = numberOfBytesInChar((unsigned char)nextChar) - 1;
					while (remainingBytes)
					{
						nextChar = fgetc(inputFilePtr);
						wordBuffer[i++] = nextChar;
						remainingBytes--;
					}
					charCount++;
					while ( (nextChar = fgetc(inputFilePtr) ) != '@')
					{
						wordBuffer[i++] = nextChar;
						remainingBytes = numberOfBytesInChar((unsigned char)nextChar) - 1;
						while (remainingBytes)
						{
							nextChar = fgetc(inputFilePtr);
							wordBuffer[i++] = nextChar;
							remainingBytes--;
						}
						charCount++;
					}
					wordBuffer[i++] = nextChar;
					remainingBytes = numberOfBytesInChar((unsigned char)nextChar) - 1;
					while (remainingBytes)
					{
						nextChar = fgetc(inputFilePtr);
						wordBuffer[i++] = nextChar;
						remainingBytes--;
					}
					charCount++;
					break;
				}
			default:
				wordBuffer[i++] = nextChar;
				remainingBytes = numberOfBytesInChar((unsigned char)nextChar) - 1;
				while (remainingBytes)
				{
					nextChar = fgetc(inputFilePtr);
					wordBuffer[i++] = nextChar;
					remainingBytes--;
				}
				charCount++;
				consecutiveNewLineCharacters = 0;
				break;
		}
				
		nextChar = fgetc(inputFilePtr);
    }
	
	*wordsCount = j;
	return 0;		//means EOF is reached
}


int main(int argc, char **argv)
{
	int handleAtSignIsEnabled;
	if (argc > 1)
	{
		handleAtSignIsEnabled = 1;		
		charsPerLine = atoi(argv[1]);
	}
	else
	{
		fprintf(stderr, "Please provide the desired number of characters per line\n");
		fprintf(stderr, "Example ./tf 100 for 100 characters per line\n");
		exit(EXIT_SUCCESS);
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

	/* Read 1st paragraph from randomText.txt */
	int wordsCount;
	int paragraphsRemain = readParagraph(inputFilePtr, &wordsCount, handleAtSignIsEnabled);

	while (paragraphsRemain)
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
		
	//Calculate the cost/badness for each possible string of words that can fit in one line.
		if (wordsCount == 0) exit(EXIT_SUCCESS);
		
		int curLineLength = 0, curLineStart, nextLineStart, currentCost, suffixBadness=0;
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
		badness = malloc( (wordsCount+1) * sizeof(int));
		lineBreaks = malloc( (wordsCount+1) * sizeof(int));
	 
		for (i=0; i < wordsCount; ++i)
		{
			for (j=0; j < wordsCount; ++j)  costBuffer[i][j] = whitespace[i][j] = INFINITY;
			badness[i] = lineBreaks[i] = INFINITY;
		}
		badness[i] = lineBreaks[i] = INFINITY;
		
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
				//else    continue;
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
		for (; curLineStart >= 0 ; --curLineStart)		//where to start building the line
		{
			while (curLineStart < nextLineStart)
			{
				if (costBuffer[curLineStart][nextLineStart-1] != INFINITY)	//the current set of  words fits in a line 
				{
					suffixBadness = (nextLineStart == wordsCount) ? 0 : badness[nextLineStart];	//the best i can do after the specified word
					
					
					if (nextLineStart == wordsCount)	currentCost=0;		//new lines
					else	currentCost = costBuffer[curLineStart][nextLineStart-1] + suffixBadness;
					
					
					if (currentCost < badness[curLineStart] || badness[curLineStart] == INFINITY)
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
		}
		
		
		i = 0;
		j = lineBreaks[i];
		int curLineWords = j - i, padDistance = 0, k = 0;			//number of words that will be printed in the current line
		int curLineSpacePositions = curLineWords -1;				//the number of positions between words that can be padded with spaces
		//while (j != INFINITY)
		while (1)
		{
			if (whitespace[i][j-1] > 0 && curLineWords > 1)
			{
				while (whitespace[i][j-1] >= curLineSpacePositions)
				{
					for (k = i; k < i+curLineSpacePositions; k++)		wordsBuffer[k].paddedSpaces++;
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
				//if (i == j-1)	fprintf(outputFilePtr, "%.*s", wordsBuffer[i].wordLength -1, wordsBuffer[i].word);
				if (i == j-1)	fprintf(outputFilePtr, "%s", wordsBuffer[i].word);
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
			if (j == wordsCount || j == -1)		//j==-1 suggests there is only one line, but better check it again.
			{
				fprintf(outputFilePtr, "\n");
				for( ; i < j; ++i)	fprintf(outputFilePtr, "%s", wordsBuffer[i].word);
				break;
			}
			
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
		
		
		paragraphsRemain = readParagraph(inputFilePtr, &wordsCount, handleAtSignIsEnabled);
	}


	fclose(outputFilePtr);
    fclose(inputFilePtr);	
    return EXIT_SUCCESS;
}



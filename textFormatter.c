#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#define MAX_WORDS 3000
#define MAX_WORD_LENGTH 150
//#define CHARS_PER_LINE 101


#define DEBUG 0
int numberOfBytesInChar(unsigned char val);


typedef struct wordDetails
{
    char * word;
    int wordLength;
	int paddedSpaces;
} wordDetails;

static int INFINITY = -1;
static int FINISHED = 0;
static int CHARS_PER_LINE;

int main(int argc, char **argv)
{
	
	if (argc == 1)
	{
		fprintf(stderr, "Please provide the desired number of characters per line\n");
		fprintf(stderr, "Example ./tf 100\n");
		return EXIT_FAILURE;
	}
	else	CHARS_PER_LINE = atoi(argv[1]);
		
    //setlocale(LC_ALL,'gr_GR.UTF-8', 'el_GR.utf8');
    //setlocale(LC_ALL,"gr_GR.UTF-8");
    FILE *inputFilePtr, *outputFilePtr;
    inputFilePtr = outputFilePtr = NULL;
    //if ((inputFilePtr = fopen("C:\\Users\\d-pkaltzias\\Desktop\\randomText.txt", "r") ) == NULL )
    if ((inputFilePtr = fopen("randomText.txt", "r") ) == NULL )
    {
        fprintf(stderr, "File error 1\n");
        exit(EXIT_FAILURE);
    }
    //if ((outputFilePtr = fopen("C:\\Users\\d-pkaltzias\\Desktop\\formattedText.txt", "w") ) == NULL )
    if ((outputFilePtr = fopen("formattedText.txt", "w") ) == NULL )
    {
        fprintf(stderr, "File error 2\n");
        exit(EXIT_FAILURE);
    }

    char wordBuffer[MAX_WORD_LENGTH];
    memset(wordBuffer, 0, MAX_WORD_LENGTH);
    wordDetails wordsBuffer[MAX_WORDS];
    int nextChar, charCount, i, j;
    i = j = charCount = 0;
    for (i=0; i < MAX_WORDS; i++)
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
	i = 0;
    while( isspace(nextChar = fgetc(inputFilePtr)) )	continue;
    //while( (nextChar = fgetc(inputFilePtr)< '0' || nextChar > '9') && (nextChar < 'A' || nextChar > 'Z') && (nextChar < 'a' || nextChar > 'z') && nextChar != ' ')	continue;
    do
    {
        //if (nextChar == '\n')    continue;
		
		if (nextChar == ' ' || nextChar == '\n')
        {
			while( isspace(nextChar = fgetc(inputFilePtr)) )	continue;
			
			//else
			//{
				//wordBuffer[i++] = nextChar;
				wordBuffer[i++] = ' ';
				wordBuffer[i] = '\0';
				wordsBuffer[j].word = malloc( (i+1) * sizeof(char) );
				strncpy(wordsBuffer[j].word, wordBuffer, i+1);
				wordsBuffer[j++].wordLength = charCount+1;
				charCount = i = 0;
			//}
			//while( isspace(nextChar = fgetc(inputFilePtr)) )	continue;
			if (nextChar == EOF)
			{
				//wordBuffer[i] = '\0';
				FINISHED = 1;
				//wordsBuffer[j].word)+
				break;
			}
        }
		
		wordBuffer[i++] = nextChar;
		int remainingBytes = numberOfBytesInChar((unsigned char)nextChar) - 1;
		while (remainingBytes)
		{
			nextChar = fgetc(inputFilePtr);
			wordBuffer[i++] = nextChar;
			remainingBytes--;
		}
		charCount++;
		if (nextChar == '@')
		{
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
		}
		if (j == MAX_WORDS)	return EXIT_FAILURE;
    }   while( (nextChar = fgetc(inputFilePtr)) != EOF);
	
	if (!FINISHED)
	{
		wordBuffer[i++] = ' ';
		wordBuffer[i] = '\0';
		//wordsBuffer[j].word = malloc( (i+2) * sizeof(char) );
		wordsBuffer[j].word = malloc( (i+1) * sizeof(char) );
		//strncpy(wordsBuffer[j].word, wordBuffer, i+2);
		strncpy(wordsBuffer[j].word, wordBuffer, i+1);
		wordsBuffer[j++].wordLength = charCount+1;
	}
	
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
	int wordsCount = j, charsPerLine = CHARS_PER_LINE;
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

	fclose(outputFilePtr);
    fclose(inputFilePtr);	
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
    return EXIT_SUCCESS;
}




int numberOfBytesInChar(unsigned char val) {
    if (val < 128) {
        return 1;
    } else if (val < 224) {
        return 2;
    } else if (val < 240) {
        return 3;
    } else {
        return 4;
    }
}

//JUNK
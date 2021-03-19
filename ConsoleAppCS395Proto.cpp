#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>

//Remember to test with release mode, should go faster
using namespace std;
//The following constants establishes sizes for the pruned data files
const int indexSize = 8;//Byte encoding for the index (8 bytes per line)
const int stateSize = 32;//Byte encoding for the state binary file (1 byte for all 32 fluents)
const int temporalLines = 16851945;//How many states there are, somewhat erroneuosly named (it is technically the number of unique lines in the temporal file)
const int actionSize = 282;//How many actions there are
const int temporalSize = 24;//How wide each line of the binary encoded temporal file is
const int temporalChunk = 8;//How wide each individual value is in each line of the binary encoded temporary file

//This takes in a certain state and outputs what line of the temporal file it is on
long long int searchIndex(int integerState) {
	long long int byteEncodingSize = (long long int)indexSize;
	//printf("\n");
	long long int state = long long int(integerState);
	long long int digit[indexSize];//this is the array of lli values, since we are banging our head against a wall
	char bufferS[indexSize];//this is the signed buffer, since we have to read in as signed char
	unsigned char buffer[indexSize];//this is the unsigned buffer

	ifstream fin("temporal_index_8byte.bin", ifstream::binary);

	//printf("Seeking to: %lli \n", integerState * byteEncodingSize);
	if (!fin.seekg(state * byteEncodingSize)) { printf("State: %i Error SeekG \n", integerState); }//skip number of bytes times the number of states
	if (!fin.read(bufferS, byteEncodingSize)) { printf("State: %lli Error Read \n", state); }//read number of bytes bytes

	//This loop converts sign char to unsigned char
	for (int i = 0; i < indexSize; ++i) {
		buffer[i] = (unsigned char)bufferS[i];
	}

	//This loop converts to long long int
	for (int i = 0; i < indexSize; ++i) {
		digit[i] = (long long int)buffer[i];

	}
	//printf("For state %d: \n", integerState);
	//cout << "Adding up the contents of the buffer gives us: \n";

	long long int sum = 0;
	long long int multiplier = 0;
	for (int i = 0; i < indexSize; i++) {
		multiplier = (long long int)1 << ((long long int)indexSize * ((long long int)(indexSize - 1) - (long long int)i));
		sum = sum + digit[i] * multiplier;
		//cout << digit[i] * multiplier << " + ";
	}
	//cout << endl;
	//printf("%lli\n", sum);
	//printf("That means that at line %d of the index file, is the number %lli \nThat tells us where to find the first line containing state %d within the pruned_temporal binary file\n", integerState, sum, integerState);
	//printf("State %d starts on line %lli\n", integerState, sum);
	//printf("\n");
	return sum;//Placeholder until buffer looks right

}

//This takes in a certain state and an empty array and then populates the array with the fluent values for that state
void searchState(int integerState, long long int searchFluents[]) {
	long long int byteEncodingSize = (long long int)stateSize;
	//printf("\n");
	long long int state = long long int(integerState);
	long long int digit[stateSize];//this is the array of lli values, since we are banging our head against a wall
	char bufferS[stateSize];//this is the signed buffer, since we have to read in as signed char
	unsigned char buffer[stateSize];//this is the unsigned buffer

	ifstream fin("binary_pruned_states.bin", ifstream::binary);

	//printf("Seeking to: %lli \n", integerState * byteEncodingSize);
	if (!fin.seekg(state * byteEncodingSize)) { printf("Error SeekG \n"); }//skip number of bytes times the number of states
	if (!fin.read(bufferS, byteEncodingSize)) { printf("Error Read \n"); }//read number of bytes bytes

	//This loop converts sign char to unsigned char
	for (int i = 0; i < stateSize; ++i) {
		buffer[i] = (unsigned char)bufferS[i];
	}

	//This loop converts to long long int
	for (int i = 0; i < stateSize; ++i) {
		digit[i] = (long long int)buffer[i];
		searchFluents[i] = digit[i];
		printf("%lli, ", searchFluents[i]);
	}

	printf("\n");
	return;
}

//This function takes in a certain state and returns how many lines it takes up in the temporal file AKA the number of outgoing graph edges AKA the number of possible actions
long long int findLineDiff(int integerState) {
	long long int startLine = searchIndex(integerState);//Find where in the file the data starts
	long long int endLine = searchIndex(integerState + 1);//Find where in the file the data ends
	//printf("%i: %lli - %lli = %lli \n", integerState, endLine, startLine, endLine - startLine);
	return endLine - startLine;//For now, just print the difference between the two 
}

//This finds the maximum difference between lines. The theoretical maximum is the number of total possible actions, so this code isn't used anymore. Included only for debugging purposes.
long long int findMaxLineDiff() {
	long long int maxDiff = 0;
	long long int tempDiff = 0;
	for (int i = 0; i < 1000; i++) {
		long long int tempDiff = findLineDiff(i);
		if (tempDiff > maxDiff) {
			maxDiff = tempDiff;
		}
	}

	return maxDiff;
}

//This function takes in a certain state and two empty arrays. It fills those arrays with the possible actions and corresponding states.
void searchTemporal(int integerState, long long int searchActions[], long long int searchStates[]) {
	int lineDiff = findLineDiff(integerState);
	int action[100];//placeholder until we find out what the max is
	int nextState[100];//....
	int doubleArray[100][2];//So we can return the actions and states together

	long long int state = long long int(integerState);
	long long int digit[100];
	char bufferS[100];
	unsigned char buffer[100];
	int byteEncodingSize = temporalSize;

	ifstream fin("binary_pruned_temporal8byte.bin", ifstream::binary);

	int actualLine = searchIndex(integerState);

	char byteAction[8];
	char byteNextState[8];
	int k = 0;//so we can remember how many lines we've read
	for (int j = actualLine; j < actualLine + lineDiff; j++) {//From the line we start with to the line at the end
		fin.seekg((long long int)j * byteEncodingSize);//skip number of bytes times the number of states
		fin.read(bufferS, byteEncodingSize);//read number of bytes bytes


		long long int actionSum = 0;
		long long int stateSum = 0;
		long long int multiplier = 0;

		for (int i = 0; i < temporalSize; ++i) {
			buffer[i] = (unsigned char)bufferS[i];
		}

		for (int i = 0; i < temporalSize; i++) {
			digit[i] = (long long int)buffer[i];
			//cout << digit[i] << " ";
			//if (i == 7 || i == 15)
				//cout << endl;
		}
		//printf("\n");
		//printf("\n");
		for (int i = 0; i < temporalChunk; i++) {
			multiplier = (long long int)1 << ((long long int)temporalChunk * ((long long int)(temporalChunk - 1) - (long long int)i));
			actionSum = actionSum + (digit[i + 8] * multiplier);
			stateSum = stateSum + (digit[i + 16] * multiplier);
			//cout <<  actionSum << " + ";
		}
		searchActions[k] = actionSum;
		searchStates[k] = stateSum;

		k++;
	}

	return;//Placeholder until buffer looks right
}

//The repeatable game loop. 
int gameLoop(long long int nextS, long long int tempA[], long long int tempS[], bool badI, string acts[], int boof, int biff)
{
	searchTemporal(nextS, tempA, tempS);//Find the possible actions and corresponding states for the state that gameLoop was given
	int actionChoice;
	int actionChoice2;
	string currentAction;
	//for (int i = 0; i < actionSize; i++) {
	//	if (tempA[i] != 0 || tempS[i] != 0)
	//		printf("Action[%i]: %lli and State[%i]: %lli\n", i, tempA[i], i, tempS[i]);
	//}

	//Until the player inputs something bad
	while (badI == true)
	{
		cout << "----------------------------------------------------------------" << endl;
		cout << "CURRENT STATE: STATE " << nextS << endl;
		cout << "----------------------------------------------------------------" << endl;
		cout << endl;
		cout << "Possible Actions: ";
		for (int i = 0; i < actionSize; i++) {
			if (tempA[i] != 0 || tempS[i] != 0)
				cout << tempA[i] << " ";
		}
		cout << endl;
		cout << endl;
		for (int i = 0; i < actionSize; i++) {
			boof = tempA[i];
			if (tempA[i] != 0 || tempS[i] != 0)
				cout << "Action Description (" << tempA[i] << "): " << acts[boof] << endl;
		}
		cout << endl;
		cout << "Please Input an Action from the Given List." << endl;
		cin >> actionChoice;
		cout << endl;
		for (int i = 0; i < 282; i++)
		{
			if (actionChoice == i)
			{
				actionChoice2 = actionChoice;
			}
		}
		actionChoice = 0;
		cin.clear();
		cin.ignore();
		for (int i = 0; i < actionSize; i++) {
			if (actionChoice2 == tempA[i] && actionChoice2 != 0 && actionChoice2 < 283)
			{
				badI = false;
			}
		}
		system("CLS");
	}


	for (int c = 0; c < actionSize; c++)
	{
		if (tempA[c] == actionChoice2)
		{
			nextS = tempS[c];
			biff = tempA[c];
		}
	}

	currentAction = acts[biff];

	cout << "----------------------------------------------------------------" << endl;

	//Check the action and print out the correct English translation of what it means
	if (currentAction[0] == 'a')
	{
		cout << "Format: attack(Attacker, Attacked, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'b')
	{
		cout << "Format: buy(Buyer, ItemPurchased, Seller, MoneyUsed, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'l')
	{
		cout << "Format: loot(Looter, ItemLooted, Victim, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'r' && currentAction[1] == 'e')
	{
		cout << "Format: report(Reporter, Guard, BanditLocation, ReporterLocation)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'r' && currentAction[1] == 'o')
	{
		cout << "Format: rob(Robber, ItemStolen, Victim, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 't')
	{
		cout << "Format: take-out(Looter, Item, Container, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'w')
	{
		cout << "Format: walk(Walker, From, To)" << endl;
		cout << "Action: " << currentAction << endl;
	}

	badI = true;
	gameLoop(nextS, tempA, tempS, badI, acts, boof, biff);

	return 0;
}

void main() {
	ifstream fluentsFile;
	string fluents[32];

	ifstream valuesFile;
	string values[16];

	ifstream actionsFile;
	string actions[282];
	string currentAction;
	long long int nextState;
	bool badInput = true;
	int actionChoice;
	int actionChoice2;
	int biff;
	int boof;

	char separators[] = "∩╗┐ ,\t\n";
	char* token1, * token2, * next_token1, * next_token2;

	long long int tempFluents[stateSize] = { 0 };
	long long int tempActions[actionSize] = { 0 };
	long long int tempStates[actionSize] = { 0 };

	//Open and store the fluents file
	fluentsFile.open("pruned_fluents.txt");
	if (fluentsFile.is_open()) {
		string tp;
		int line = 0;
		while (getline(fluentsFile, tp)) {
			fluents[line] = tp;
			line++;
		}

		fluentsFile.close();
	}

	//Open and store the values file
	valuesFile.open("pruned_values.txt");
	if (valuesFile.is_open()) {
		string tp;
		int line = 0;
		while (getline(valuesFile, tp)) {
			values[line] = tp;
			line++;
		}
		valuesFile.close();

	}

	//Open and store the actions file
	actionsFile.open("pruned_actions.txt");
	if (actionsFile.is_open()) {
		string tp;
		int line = 0;
		while (getline(actionsFile, tp)) {
			actions[line] = tp;
			line++;
		}
		actionsFile.close();

	}

	//Find the possible actions and corresponding states for state 0
	searchTemporal(0, tempActions, tempStates);

	//for (int i = 0; i < actionSize; i++) {
	//	if (tempActions[i] != 0 || tempStates[i] != 0)
	//		printf("Action[%i]: %lli and State[%i]: %lli\n", i, tempActions[i], i, tempStates[i]);
	//}

	//While the user hasn't input a faulty action
	while (badInput == true)
	{
		system("CLS");//Clear screen
		cout << "----------------------------------------------------------------" << endl;
		cout << "BEGINNING STATE: STATE 0" << endl;
		cout << "----------------------------------------------------------------" << endl;
		cout << "Possible Actions: ";

		//Print out possible actions
		for (int i = 0; i < actionSize; i++) {
			if (tempActions[i] != 0 || tempStates[i] != 0)
				cout << tempActions[i] << " ";
		}
		cout << endl;
		cout << endl;

		//Print out English descriptions of the actions
		for (int i = 0; i < actionSize; i++) {
			boof = tempActions[i];
			if (tempActions[i] != 0 || tempStates[i] != 0)
				cout << "Action Description (" << tempActions[i] << "): " << actions[boof] << endl;
		}
		cout << endl;
		cout << "Please Input an Action from the Given List." << endl;

		//Take player input
		cin >> actionChoice;
		cout << endl;

		//Make sure the chosen action works
		for (int i = 0; i < 282; i++)
		{
			if (actionChoice == i)
			{
				actionChoice2 = actionChoice;//Only allow the user input through if it is a possible action
			}
		}
		actionChoice = 0;
		cin.clear();
		cin.ignore();
		for (int i = 0; i < actionSize; i++) {
			if (actionChoice2 == tempActions[i] && actionChoice2 != 0 && actionChoice2 < 283)//So long as the action is legal...
			{
				badInput = false;//This means the player typed an action wrong
			}
		}
		system("CLS");
	}

	for (int c = 0; c < actionSize; c++)//For every possible action
	{
		if (tempActions[c] == actionChoice2)//If the list of actions matches the valid player-selected action
		{
			nextState = tempStates[c];//The next state is the corresponding state from the state array that matches the valid player selected action
			biff = tempActions[c];//This is now the chosen action
		}
	}

	currentAction = actions[biff];//Set the current action

	cout << "----------------------------------------------------------------" << endl;

	//The following code is a series of if statements to print out the correct English explanation for each of the different types of actions
	if (currentAction[0] == 'a')
	{
		cout << "Format: attack(Attacker, Attacked, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'b')
	{
		cout << "Format: buy(Buyer, ItemPurchased, Seller, MoneyUsed, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'l')
	{
		cout << "Format: loot(Looter, ItemLooted, Victim, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'r' && currentAction[1] == 'e')
	{
		cout << "Format: report(Reporter, Guard, BanditLocation, ReporterLocation)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'r' && currentAction[1] == 'o')
	{
		cout << "Format: rob(Robber, ItemStolen, Victim, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 't')
	{
		cout << "Format: take-out(Looter, Item, Container, Location)" << endl;
		cout << "Action: " << currentAction << endl;
	}
	else if (currentAction[0] == 'w')
	{
		cout << "Format: walk(Walker, From, To)" << endl;
		cout << "Action: " << currentAction << endl;
	}

	badInput = true;//The loop may continue
	gameLoop(nextState, tempActions, tempStates, badInput, actions, boof, biff);//Run the game loop

}

//Old debugging code, not currently utilized
int errorcheckmain() {
	long long int tempFluents[stateSize] = { 0 };
	long long int tempActions[actionSize] = { 0 };
	long long int tempStates[actionSize] = { 0 };

	srand(time(NULL));
	int nextState(int now, int input);

	//searchIndex(rand() % 16851944);


	for (int i = 0; i < actionSize; i++) {
		if (tempActions[i] != 0 || tempStates[i] != 0)
			printf("Action[%i]: %lli and State[%i]: %lli\n", i, tempActions[i], i, tempStates[i]);
	}

	/*
	for (int j = 0; j < 16851944; j++) {
		searchTemporal(j, tempActions, tempStates);
		for (int i = 0; i < actionSize; i++) {
			if (tempActions[i] == -3689348814741910324)
				printf("%i\n", j);
				//printf("Action[%i]: %lli and State[%i]: %lli\n", i, tempActions[i], i, tempStates[i]);
		}
	}
	*/

	//cout << searchIndex(1) << endl;
	//nextState(currentState, actionChoice);

	return 0;
}

//Legacy code, only left as a reminder of the past.
int nextState(int now, int input) {
	int x = now;
	string strX = to_string(x);
	int y = input;
	string strY = to_string(y);
	int z = 0;
	int stringItter = 0;
	string checker;
	bool loopclear = false;
	/*
	while (loopclear == false)
	{
		checker = TemporalArray[z];
		if (checker[stringItter] == strX[stringItter])
		{

		}
	}
	*/
	cout << "Current State: " << x << endl;
	cout << "Action Taken: " << y << endl;
	cout << "Moving to State: " << "NULL" << endl; //This is the current goal of building repeating this function.

	return 0; //Will return a recursive function in next step.
}
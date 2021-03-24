/*   Author 
 * First Name: Teodor Ioan
 * Last Name: Hossu
 * Group: 315CB
 */

#include <stdio.h>

// ----------------------------------------------------------------------------
// Structure declarations

typedef struct History {
	char *status;
	char *command;
	char *extras;
	struct History *next;
} History;

typedef struct Card {
	char cardNumber[17];
	char pin[5];
	char expiryDate[6];
	char cvv[4];
	long long balance;
	char *status;
	int attempts;
	History *history;
	struct Card *next;
} LSC;

typedef struct LC {
	LSC *cards;
	struct LC *next;
} LC;

// ----------------------------------------------------------------------------
// Macro declarations

#define NO(c) (c->cardNumber)
#define PIN(c) (c->pin)
#define CVV(c) (c->cvv)
#define EXP(c) (c->expiryDate)
#define BAL(c) (c->balance)
#define STS(c) (c->status)
#define ATT(c) (c->attempts)
#define HIST(c) (c->history)

// ----------------------------------------------------------------------------
// Function declarations

LSC *createCard(char *cardNumber, char *pin, char *expiryDate, char *cvv);

int sum(char *cardNumber);

int cardPosition(char *cardNumber, int noMaxCards);

LSC *cardExistsInList(LSC *cardsList, char *cardNumber);

int addCard(LC **list, int pos, LSC *card);

// -------------------

void deleteCard(LC *list, char *cardNumber);

// -------------------

void showHistory(History *history, FILE *out);

void showCard(LSC *card, FILE *out);

void showCards(LSC *cards, FILE *out);

void showATM(LC *list, FILE *out, char *cardNumber);

// -------------------

void addToHistory(LSC *card, char *status, char *command, char *extras);

// -------------------

LSC *insertCard(LC *list, char *cardNumber, char *pin, FILE *out);

// -------------------

void unblockCard(LC *list, char *cardNumber);

// -------------------

int isNumber(char *string);

int pinChange(LSC *card, char *pin);

// -------------------

int balanceInquiry(LSC *card);

// -------------------

int recharge(LSC *card, long long value);

// -------------------

int cashWithdrawal(LSC *card, long long value);

// -------------------

int transferFunds(LC *list, LSC *activeCard, char *cardNumberReceiver,
	long long value);

// -------------------

int reverseTransaction(LC *list, char *sender, char *receiver,
	long long value);

// -------------------

void recursiveAdd(LSC *card, LC **newList, int size);

void recursiveListAdd(LC *list, LC **newList, int size);

LC *resizeATM(LC *list, int *noMaxCards);

// -------------------

void freeHistory(History *history);

void freeHistories(History *history);

void freeCard(LSC *card);

void freeCards(LSC *cards);

void freeATM(LC *list);

// -------------------
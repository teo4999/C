/*  Author 
 * First Name: Teodor Ioan
 * Last Name: Hossu
 * Group: 315CB
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atm.h"

/**
 *	Creates a new card with the given data.
 */
LSC *createCard(char *cardNumber, char *pin, char *expiryDate, char *cvv) {
	LSC *newCard = calloc(1, sizeof(LSC));
	if (!newCard) {
		printf("Memory failure!\n");
		exit(EXIT_FAILURE);
	}
	strcpy(NO(newCard), cardNumber);
	strcpy(PIN(newCard), pin);
	strcpy(EXP(newCard), expiryDate);
	strcpy(CVV(newCard), cvv);
	STS(newCard) = strdup("NEW");
	return newCard;
}

/**
 *	Calculates the sum of the digits of a card number.
 */
int sum(char *cardNumber) {
	int sum = 0, i = 0;
	while(i < 16) {
		sum += (cardNumber[i] - '0') % 10;
		i++;
	}
	return sum;
}

/**
 *	Calculates the position the new card will be put in into the ATM
 */
int cardPosition(char *cardNumber, int noMaxCards) {

	return sum(cardNumber) % noMaxCards;
}

/**
 *	Returns the previous of the card with the given card number or null.
 */
LSC *cardExistsInList(LSC *cardsList, char *cardNumber) {
	LSC *previous = cardsList;
	while (cardsList) {
		if (!strcmp(cardNumber, NO(cardsList))) {
			return previous;
		}
		previous = cardsList;
		cardsList = cardsList->next;
	}
	return NULL;
}

int cardExists(LC *list, char *cardNumber) {
	while (list) {
		if (cardExistsInList(list->cards, cardNumber)) {
			return 1;
		}
		list = list->next;
	}
	return 0;
}

/**
 *	Adds a new card to the ATM lists.
 */
int addCard(LC **list, int pos, LSC *card) {
	int count = 0;

	if (*list == NULL) {
		*list = calloc(1, sizeof(LC));
		if (!list) {
			printf("Memory failure!\n");
			exit(EXIT_FAILURE);
		}
		if (pos == 0) {
			(*list)->cards = card;
			return 0;
		}
	}

	// Searches the proper list for the new card
	LC *current = *list;
	while (count != pos) {
		if (current->next == NULL) {
			current->next = calloc(1, sizeof(LC));
			if (!current->next) {
				printf("Memory failure!\n");
				exit(EXIT_FAILURE);
			}
		}
		current = current->next;
		count++;
	}

	// Adds the card at the beggining of the list
	if (!cardExistsInList(current->cards, NO(card))) {
		card->next = current->cards;
		current->cards = card;
		return 0;
	}
	freeCard(card);
	return 1;
}

/**
 *	Deletes a card from the ATM database.
 */
void deleteCard(LC *list, char *cardNumber) {
	LSC *previous;
	while (list) {
		if ((previous = cardExistsInList(list->cards, cardNumber))) {
			LSC *toDelete;
			if (!strcmp(NO(previous), cardNumber)) {
				toDelete = previous;
				list->cards = list->cards->next;
			} else {
				toDelete = previous->next;
				previous->next = toDelete->next;
			}
			freeCard(toDelete);
			return;
		}
		list = list->next;
	}
}

/**
 *	Prints the history of a card.
 */
void showHistory(History *history, FILE *out) {
	while (history) {
		fprintf(out, "(%s, ", history->status);
		fprintf(out, "%s %s)", history->command, history->extras);
		if (history->next) fprintf(out, ", ");
		history = history->next;
	}
}

/**
 *	Prints the details of a card.
 */
void showCard(LSC *card, FILE *out) {
	fprintf(out, "(card number: %s, ", NO(card));
	fprintf(out, "PIN: %s, expiry date: %s, CVV: %s, ", PIN(card),
		EXP(card), CVV(card));
	fprintf(out, "balance: %lld, status: %s, ", BAL(card), STS(card));
	fprintf(out, "history: [");
	showHistory(HIST(card), out);
	fprintf(out, "])\n");
}

/**
 *	Prints the cards from a list.
 */
void showCards(LSC *cards, FILE *out) {
	while (cards) {
		showCard(cards, out);
		cards = cards->next;
	}
}

/**
 *	Prints the ATM database or the details of a single card from it.
 */
void showATM(LC *list, FILE *out, char *cardNumber) {
	int counter = 0;
	while (list) {
		if (cardNumber == NULL) {
			fprintf(out, "pos%d: [", counter);

			if (list->cards) {
				fprintf(out, "\n");
				showCards(list->cards, out);
			}

			fprintf(out, "]\n");
			counter++;
		} else {
			LSC *card;
			if ((card = cardExistsInList(list->cards, cardNumber))) {
				if (strcmp(NO(card), cardNumber)) {
					card = card->next;
				}
				showCard(card, out);
				return;
			}
		}
		list = list->next;
	}
}

/**
 *	Adds a command to the history of a card.
 */
void addToHistory(LSC *card, char *status, char *command, char *extras) {
	History *newHistory = calloc(1, sizeof(History));
	if (!newHistory) {
		printf("Memory failure!\n");
		exit(EXIT_FAILURE);
	}
	newHistory->status = strdup(status);
	newHistory->command = strdup(command);
	newHistory->extras = strdup(extras);
	newHistory->next = HIST(card);
	HIST(card) = newHistory;
	free(extras);
}

/**
 *	Inserts a card into the ATM.
 */
LSC *insertCard(LC *list, char *cardNumber, char *pin, FILE *out) {
	char *buffer = calloc(100, sizeof(char));
	if (!buffer) {
		printf("Memory failure!\n");
		exit(EXIT_FAILURE);
	}
	strcat(buffer, cardNumber);
	strcat(buffer, " ");
	strcat(buffer, pin);
	LSC *card;
	while (list) {
		if ((card = cardExistsInList(list->cards, cardNumber))) {
			if (strcmp(NO(card), cardNumber)) {
				card = card->next;
			}
			// If the card is locked from previous failed login attempts
			if (!(strcmp(STS(card), "LOCKED"))) {
				fprintf(out, "The card is blocked. ");
				fprintf(out, "Please contact the administrator.\n");

				addToHistory(card, "FAIL", "insert_card", buffer);
				return NULL;
			}
			// If the PIN is different than that of the card
			if (strcmp(PIN(card), pin)) {
				ATT(card)++;
				fprintf(out, "Invalid PIN\n");
				if (ATT(card) == 3) {
					fprintf(out, "The card is blocked. ");
					fprintf(out, "Please contact the administrator.\n");
					free(STS(card));
					STS(card) = strdup("LOCKED");
				}

				addToHistory(card, "FAIL", "insert_card", buffer);
				return NULL;
			}
			// If the card has the default PIN
			if (!strcmp(STS(card), "NEW")) {
				fprintf(out, "You must change your PIN.\n");
			}

			addToHistory(card, "SUCCESS", "insert_card", buffer);
			ATT(card) = 0;
			return card;
		}
		list = list->next;
	}
	free(buffer);
	return NULL;
}

/**
 *	Unlocks a card or resets to 0 the failed attempts from inserting the card.
 */
void unblockCard(LC *list, char *cardNumber) {
	LSC *card;
	while (list) {
		if ((card = cardExistsInList(list->cards, cardNumber))) {
			if (strcmp(NO(card), cardNumber)) {
				card = card->next;
			}
			ATT(card) = 0;
			free(STS(card));
			STS(card) = strdup("ACTIVE");
		}
		list = list->next;
	}
}

/**
 *	Verifies if a string is a number.
 */
int isNumber(char *string) {
	int i, n = strlen(string);
	for (i = 0; i < n; i++) {
		if (string[i] < '0' || string[i] > '9') {
			return 0;
		}
	}
	return 1;
}

/**
 *	Changes the PIN of a card
 */
int pinChange(LSC *card, char *pin) {
	char *buffer = calloc(100, sizeof(char));
	if (!buffer) {
		printf("Memory failure!\n");
		exit(EXIT_FAILURE);
	}
	strcat(buffer, NO(card));
	strcat(buffer, " ");
	strcat(buffer, pin);
	if (strlen(pin) != 4 || !isNumber(pin)) {
		addToHistory(card, "FAIL", "pin_change", buffer);
		return 1;
	}
	free(STS(card));
	STS(card) = strdup("ACTIVE");
	strcpy(PIN(card), pin);
	addToHistory(card, "SUCCESS", "pin_change", buffer);
	return 0;
}

/**
 *	Checks the balance of a card.
 */
int balanceInquiry(LSC *card) {
	addToHistory(card, "SUCCESS", "balance_inquiry", strdup(NO(card)));
	return BAL(card);
}

/**
 *	Charges a card with a sum of money.
 */
int recharge(LSC *card, long long value) {
	char *buffer = calloc(100, sizeof(char));
	if (!buffer) {
		printf("Memory failure!\n");
		exit(EXIT_FAILURE);
	}
	strcat(buffer, NO(card));
	strcat(buffer, " ");
	char extras[20];
	sprintf(extras, "%lld", value);
	strcat(buffer, extras);
	if (value % 10 == 0) {
		addToHistory(card, "SUCCESS", "recharge", buffer);
		BAL(card) += value;
		return 0;
	} else {
		addToHistory(card, "FAIL", "recharge", buffer);
		return 1;
	}
}

/**
 *	Withdraws from the ATM a sum of money.
 */
int cashWithdrawal(LSC *card, long long value) {
	char *buffer = calloc(100, sizeof(char));
	if (!buffer) {
		printf("Memory failure!\n");
		exit(EXIT_FAILURE);
	}
	strcat(buffer, NO(card));
	strcat(buffer, " ");
	char extras[20];
	sprintf(extras, "%lld", value);
	strcat(buffer, extras);
	if (value % 10 != 0) {
		addToHistory(card, "FAIL", "cash_withdrawal", buffer);
		return 1;
	} else if (BAL(card) < value) {
		addToHistory(card, "FAIL", "cash_withdrawal", buffer);
		return 2;
	} else {
		addToHistory(card, "SUCCESS", "cash_withdrawal", buffer);
		BAL(card) -= value;
		return 0;
	}
}

/**
 *	Transfers a sum of money from the source card to the receiving card.
 */
int transferFunds(LC *list, LSC *card, char *cardNumberReceiver,
	long long value) {
	char *buffer = calloc(100, sizeof(char));
	if (!buffer) {
		printf("Memory failure!\n");
		exit(EXIT_FAILURE);
	}
	strcat(buffer, NO(card));
	strcat(buffer, " ");
	strcat(buffer, cardNumberReceiver);
	strcat(buffer, " ");
	char extras[20];
	sprintf(extras, "%lld", value);
	strcat(buffer, extras);

	if (value % 10 != 0) {
		addToHistory(card, "FAIL", "transfer_funds", buffer);
		return 1;
	} else if (BAL(card) < value) {
		addToHistory(card, "FAIL", "transfer_funds", buffer);
		return 2;
	} else {
		addToHistory(card, "SUCCESS", "transfer_funds", strdup(buffer));
		BAL(card) -= value;
		while (list) {
			if ((card = cardExistsInList(list->cards, cardNumberReceiver))) {
				if (strcmp(NO(card), cardNumberReceiver)) {
					card = card->next;
				}
				BAL(card) += value;
				addToHistory(card, "SUCCESS", "transfer_funds", buffer);
				return 0;
			}
			list = list->next;
		}
		addToHistory(card, "FAIL", "transfer_funds", buffer);
		return 0;
	}
}

/**
 *	Reverts a wrong transfer of money.
 */
int reverseTransaction(LC *list, char *sender, char *receiver,
	long long value) {
	LC *aux = list;
	LSC *cardSender;
	while (aux) {
		if ((cardSender = cardExistsInList(aux->cards, sender))) {
			if (strcmp(NO(cardSender), sender)) {
				cardSender = cardSender->next;
			}
			break;
		}
		aux = aux->next;
	}
	LSC *cardReceiver;
	while (list) {
		if ((cardReceiver = cardExistsInList(list->cards, receiver))) {
			if (strcmp(NO(cardReceiver), receiver)) {
				cardReceiver = cardReceiver->next;
			}
			break;
		}
		list = list->next;
	}

	if (BAL(cardReceiver) < value) {
		return 1;
	}
	BAL(cardSender) += value;
	BAL(cardReceiver) -= value;

	char *buffer = calloc(100, sizeof(char));
	if (!buffer) {
		printf("Memory failure!\n");
		exit(EXIT_FAILURE);
	}
	strcat(buffer, sender);
	strcat(buffer, " ");
	strcat(buffer, receiver);
	strcat(buffer, " ");
	char extras[20];
	sprintf(extras, "%lld", value);
	strcat(buffer, extras);
	History *history = HIST(cardReceiver);
	if (!strcmp(history->command, "transfer_funds") &&
		!strcmp(history->status, "SUCCESS") &&
		!strcmp(history->extras, buffer)) {
		HIST(cardReceiver) = HIST(cardReceiver)->next;
		freeHistory(history);
		addToHistory(cardSender, "SUCCESS", "reverse_transaction", buffer);
		return 0;
	}
	while (history->next) {
		if (!strcmp(history->next->command, "transfer_funds") &&
			!strcmp(history->next->status, "SUCCESS") &&
			!strcmp(history->next->extras, buffer)) {
			History *toDelete = history->next;
			history->next = history->next->next;
			freeHistory(toDelete);
			break;
		}
		history = history->next;
	}

	addToHistory(cardSender, "SUCCESS", "reverse_transaction", buffer);
	return 0;
}

/**
 *	Adds the cards of a list to the new resized list.
 */
void recursiveAdd(LSC *card, LC **newList, int size) {
	if (card) {
		LSC *newCard = calloc(1, sizeof(LSC));
		if (!newCard) {
			printf("Memory failure!\n");
			exit(EXIT_FAILURE);
		}
		strcpy(NO(newCard), NO(card));
		strcpy(PIN(newCard), PIN(card));
		strcpy(EXP(newCard), EXP(card));
		strcpy(CVV(newCard), CVV(card));
		STS(newCard) = strdup(STS(card));
		HIST(newCard) = HIST(card);
		HIST(card) = NULL;
		ATT(newCard) = ATT(card);
		BAL(newCard) = BAL(card);
		int pos = cardPosition(NO(card), size);
		addCard(newList, pos, newCard);
		recursiveAdd(card->next, newList, size);
	}
}

/**
 *	Parses the lists of cards in order to add them to the resized list.
 */
void recursiveListAdd(LC *list, LC **newList, int size) {
	if (list) {
		LSC *card = list->cards;
		recursiveAdd(card, newList, size);
		recursiveListAdd(list->next, newList, size);
		
	}
}

/**
 *	Resizes the ATM database with double the previous maximum card capacity.
 */
LC *resizeATM(LC *list, int *noMaxCards) {
	int newSize = (*noMaxCards) * 2;
	LC *newList = NULL, *aux = list;
	recursiveListAdd(list, &newList, newSize);
	LC *previous;
	while (aux) {
		previous = aux;
		LSC *aux3, *aux2 = aux->cards;
		while(aux2) {
			aux3 = aux2;
			aux2 = aux2->next;
			freeCard(aux3);
		}
		aux = aux->next;
		free(previous);
	}
	*noMaxCards = newSize;
	return newList;
}

/**
 *	Eliberates memory for a transaction.
 */
void freeHistory(History *history) {
	free(history->status);
	free(history->command);
	free(history->extras);
	free(history);
}

/**
 *	Frees the list of transactions.
 */
void freeHistories(History *history) {
	History *previous = history;
	while (history) {
		previous = history;
		history = history->next;
		freeHistory(previous);
	}
}

/**
 *	Eliberates the memory for a card.
 */
void freeCard(LSC *card) {
	freeHistories(HIST(card));
	free(STS(card));
	free(card);
}

/**
 *	Frees a list of cards.
 */
void freeCards(LSC *cards) {
	LSC *previous;
	while (cards) {
		previous = cards;
		cards = cards->next;
		freeCard(previous);
	}
}

/**
 *	Frees the whole ATM database.
 */
void freeATM(LC *list) {
	LC *previous;
	while (list) {
		previous = list;
		list = list->next;
		freeCards(previous->cards);
		free(previous);
	}
}

int main() {
	char input[] = "input.in";
	char output[] = "output.out";

	FILE *in = fopen(input, "r");
	if (!in) {
		printf("Error opening input file!\n");
		exit(EXIT_FAILURE);
	}

	LC *list = NULL;
	int noMaxCards;
	fscanf(in, "%d", &noMaxCards);


	LSC *activeCard = NULL;
	int noCards = 0;
	FILE *out = fopen(output, "w");
	if (!out) {
		printf("Error opening output file!\n");
		exit(EXIT_FAILURE);
	}
	char command[20];
	while (fscanf(in, "%s ", command) > 0) {
		if (!strcmp(command, "add_card")) {
			char cardNumber[17];
			char pin[5];
			char expiryDate[6];
			char cvv[4];
			fscanf(in, "%s %s %s %s", cardNumber, pin, expiryDate, cvv);
			if (cardExists(list, cardNumber)) {
				fprintf(out, "The card already exists\n");
				continue;
			}
			noCards++;
			if (noCards > noMaxCards) {
				list = resizeATM(list, &noMaxCards);
			}
			LSC *card = createCard(cardNumber, pin, expiryDate, cvv);
			int pos = cardPosition(cardNumber, noMaxCards);
			addCard(&list, pos, card);
		} else if (!strcmp(command, "delete_card")) {
			noCards--;
			char cardNumber[17];
			fscanf(in, "%s", cardNumber);
			deleteCard(list, cardNumber);
		} else if (!strcmp(command, "reverse_transaction")) {
			char cardNumberSender[17];
			char cardNumberReceiver[17];
			long long value;
			fscanf(in, "%s %s %lld", cardNumberSender, cardNumberReceiver,
				&value);
			if (reverseTransaction(list, cardNumberSender, cardNumberReceiver,
				value)) {
				fprintf(out, "The transaction cannot be reversed\n");
			}
		} else if (!strcmp(command, "unblock_card")) {
			char cardNumber[17];
			fscanf(in, "%s", cardNumber);
			unblockCard(list, cardNumber);
		} else if (!strcmp(command, "show")) {
			fpos_t position;
			fgetpos(in, &position);

			char *buffer = calloc(20, sizeof(char));
			if (!buffer) {
				printf("Memory failure!\n");
				exit(EXIT_FAILURE);
			}
			fscanf(in, "%s", buffer);
			long long cardNumber = atoll(buffer);
			if (cardNumber == 0) {
				fsetpos(in, &position);
				free(buffer);
				buffer = NULL;
			}
			showATM(list, out, buffer);
			free(buffer);
		} else if (!strcmp(command, "insert_card")) {
			char *cardNumber = calloc(17, sizeof(char));
			if (!cardNumber) {
				printf("Memory failure!\n");
				exit(EXIT_FAILURE);
			}
			char pin[5];
			char cardNumber2[17];
			fscanf(in, "%s %s", cardNumber, pin);
			strcpy(cardNumber2, cardNumber);
			free(cardNumber);
			activeCard = insertCard(list, cardNumber2, pin, out);
		} else if (!strcmp(command, "recharge")) {
			char cardNumber[17];
			long long value;
			fscanf(in, "%s %lld", cardNumber, &value);
			if (recharge(activeCard, value)) {
				fprintf(out, "The added amount must be multiple of 10\n");
			} else {
				fprintf(out, "%lld\n", BAL(activeCard));
			}
			activeCard = NULL;
		} else if (!strcmp(command, "cash_withdrawal")) {
			char cardNumber[17];
			long long value;
			fscanf(in, "%s %lld", cardNumber, &value);
			int code;
			if ((code = cashWithdrawal(activeCard, value)) == 1) {
				fprintf(out, "The requested amount must be multiple of 10\n");
			} else if (code == 2) {
				fprintf(out, "Insufficient funds\n");
			} else {
				fprintf(out, "%lld\n", BAL(activeCard));
			}
			activeCard = NULL;
		} else if (!strcmp(command, "balance_inquiry")) {
			char cardNumber[17];
			fscanf(in, "%s", cardNumber);
			fprintf(out, "%d\n", balanceInquiry(activeCard));
			activeCard = NULL;
		} else if (!strcmp(command, "transfer_funds")) {
			char cardNumberSender[17];
			char cardNumberReceiver[17];
			long long value;
			fscanf(in, "%s %s %lld", cardNumberSender, cardNumberReceiver,
				&value);
			int code;
			if ((code = transferFunds(list, activeCard, cardNumberReceiver,
				value)) == 1) {
				fprintf(out, "The transferred amount must be multiple of 10\n");
			} else if (code == 2) {
				fprintf(out, "Insufficient funds\n");
			} else {
				fprintf(out, "%lld\n", BAL(activeCard));
			}
			activeCard = NULL;
		} else if (!strcmp(command, "pin_change")) {
			char *cardNumber = calloc(17, sizeof(char));
			if (!cardNumber) {
				printf("Memory failure!\n");
				exit(EXIT_FAILURE);
			}
			char pin[5];
			fscanf(in, "%s %s", cardNumber, pin);
			char cardNumber2[17];
			strcpy(cardNumber2, cardNumber);
			free(cardNumber);
			if (pinChange(activeCard, pin)) {
				fprintf(out, "Invalid PIN\n");
			}
			activeCard = NULL;
		} else if (!strcmp(command, "cancel")) {
			char cardNumber[17];
			fscanf(in, "%s", cardNumber);
			addToHistory(activeCard, "SUCCESS", "cancel", strdup(cardNumber));
			activeCard = NULL;
		}
	}

	fclose(in);
	fclose(out);
	freeATM(list);
	return 0;
}

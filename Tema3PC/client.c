#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"

#define HOST "ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"
#define PORT 8080

#define URL_REGISTER "/api/v1/tema/auth/register"
#define URL_LOGIN "/api/v1/tema/auth/login"
#define URL_LOGOUT "/api/v1/tema/auth/logout"
#define URL_ACCES "/api/v1/tema/library/access"
#define URL_BOOKS "/api/v1/tema/library/books"
#define URL_BOOK "/api/v1/tema/library/books/"

struct sockaddr *get_addr(char* name)
{
	int ret;
	struct addrinfo hints, *result;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	ret = getaddrinfo(name, NULL, &hints, &result);
	if (ret != 0)
	{
		perror("getaddrinfo");
	}

	return result->ai_addr;
}

int main(int argc, char *argv[])
{
    int server_socket;

    //obtinerea IP-ului de la server
    struct sockaddr_in *serv_addr = (struct sockaddr_in *) get_addr(HOST);

    char ip[100];
    memset(ip, 0, 100);
    inet_ntop(AF_INET, &(serv_addr->sin_addr), ip, 100);

    //conectare la server
    server_socket = open_connection(ip, PORT, AF_INET, SOCK_STREAM, 0);

    int loggedin = 0;
    int entered_library = 0;
    char cookie[LINELEN];
    char JWT[BUFLEN];

    while(1) {
    	//citire comanda de la tastatura
    	char command[20];
    	fgets(command, LINELEN, stdin);
    	command[strlen(command)-1] = '\0';

    	char *message = calloc(BUFLEN, sizeof(char));
    	char *line = calloc(LINELEN, sizeof(char));

    	if(!strcmp(command, "register")) {
    		//citire username si password de la tastatura
    		char *username = calloc(LINELEN, sizeof(char));
    		char *password = calloc(LINELEN, sizeof(char));
    		printf("username=");
    		fgets(username, LINELEN, stdin);
    		username[strlen(username)-1] = '\0';
    		printf("password=");
    		fgets(password, LINELEN, stdin);
    		password[strlen(password)-1] = '\0';

    		//conectare la server
    		server_socket = open_connection(ip, PORT, AF_INET, SOCK_STREAM, 0);

    		//crearea mesajului care trebuie transmis server-ului
    		sprintf(line, "POST %s HTTP/1.1", URL_REGISTER);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Host: %s:%d", HOST, PORT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Content-Type: application/json");
    		compute_message(message, line);

    		int content_length = 29 + strlen(username) + strlen(password);
    		memset(line, 0, LINELEN);
    		sprintf(line, "Content-Length: %d", content_length);
    		compute_message(message, line);

    		compute_message(message, "");

    		memset(line, 0, LINELEN);
    		sprintf(line, "{\"username\":\"%s\",\"password\":\"%s\"}",
    		 	username, password);
    		compute_message(message, line);

    		free(line);
    		free(username);
    		free(password);

    		//trimitere la server
    		send_to_server(server_socket, message);
    		char *get_request_response = receive_from_server(server_socket);

    		//afisare raspuns de la server
    		printf("%s\n", get_request_response);

    		free(message);
    	}
    	else if(!strcmp(command, "login")) {
    		//conectare la server
    		server_socket = open_connection(ip, PORT, AF_INET, SOCK_STREAM, 0);

    		//citire username si password de la tastatura
    		char *username = calloc(LINELEN, sizeof(char));
    		char *password = calloc(LINELEN, sizeof(char));
    		printf("username=");
    		fgets(username, LINELEN, stdin);
    		username[strlen(username)-1] = '\0';
    		printf("password=");
    		fgets(password, LINELEN, stdin);
    		password[strlen(password)-1] = '\0';

    		//crearea mesajului care trebuie transmis server-ului
    		sprintf(line, "POST %s HTTP/1.1", URL_LOGIN);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Host: %s:%d", HOST, PORT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Content-Type: application/json");
    		compute_message(message, line);

    		int content_length = 29 + strlen(username) + strlen(password);
    		memset(line, 0, LINELEN);
    		sprintf(line, "Content-Length: %d", content_length);
    		compute_message(message, line);

    		compute_message(message, "");

    		memset(line, 0, LINELEN);
    		sprintf(line, "{\"username\":\"%s\",\"password\":\"%s\"}",
    		 	username, password);
    		compute_message(message, line);

    		free(line);
    		free(username);
    		free(password);

    		//trimitere la server
    		send_to_server(server_socket, message);
    		char *get_request_response = receive_from_server(server_socket);

    		//afisare raspuns de la server
    		printf("%s\n", get_request_response);

    		char status[LINELEN];
    		char *start = strstr(get_request_response, "HTTP/1.1 ");
    		char *end = strstr(start, "\n");
    		strncpy(status, start, end - start);

    		free(message);

    		//verificare status (daca s-a logat sau nu)
    		if(strcmp(status, "HTTP/1.1 200 OK") == 13) {
    			memset(cookie, 0, LINELEN);

    			//memorare cookie
    			char *start = strstr(get_request_response, "Set-Cookie: ");
    			start += strlen("Set-Cookie: ");
    			char *end = strstr(start, ";");
    			strncpy(cookie, start, end - start);

    			loggedin = 1;
    		}
    	}
    	else if(!strcmp(command, "enter_library")) {
    		if(loggedin == 0)
    			printf("Trebuie sa fii logat!\n");
    		else if(entered_library == 1)
    			printf("Ai deja acces la biblioteca!\n");
    		else {
    		//conectare la server
    		server_socket = open_connection(ip, PORT, AF_INET, SOCK_STREAM, 0);

    		//crearea mesajului care trebuie transmis server-ului
    		sprintf(line, "GET %s HTTP/1.1", URL_ACCES);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Host: %s:%d", HOST, PORT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Cookie: %s\r\n", cookie);
    		compute_message(message, line);

    		free(line);

    		//trimiterea mesajului la server
    		send_to_server(server_socket, message);
    		char *get_request_response = receive_from_server(server_socket);

    		//afisarea mesajului de la server
    		printf("%s\n\n", get_request_response);

    		free(message);

    		//memorare JWT
    		JWT[0] = 0;

    		char *start = strstr(get_request_response, "{\"token\":\"");
    		start += strlen("{\"token\":\"");
    		char *end = strstr(start, "\"");
    		strncpy(JWT, start, end - start);

    		entered_library = 1;
    		}
    	}
    	else if(!strcmp(command, "get_books")) {
    		if(entered_library == 0)
    			printf("Trebuie sa ai acces la biblioteca!\n");
    		else {
    		server_socket = open_connection(ip, PORT, AF_INET, SOCK_STREAM, 0);

    		sprintf(line, "GET %s HTTP/1.1", URL_BOOKS);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Host: %s:%d", HOST, PORT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Authorization: Bearer %s\r\n", JWT);
    		compute_message(message, line);

    		free(line);

    		send_to_server(server_socket, message);
    		char *get_request_response = receive_from_server(server_socket);
    		printf("%s\n\n", get_request_response);

    		free(message);
    		}
    	}
    	else if(!strcmp(command, "get_book")) {
    		if(entered_library == 0)
    			printf("Trebuie sa ai acces la biblioteca!\n");
    		else {
    		char id[LINELEN];

    		printf("id=");
    		fgets(id, LINELEN, stdin);
    		id[strlen(id)-1] = '\0';
    		
    		server_socket = open_connection(ip, PORT, AF_INET, SOCK_STREAM, 0);

    		sprintf(line, "GET %s%s HTTP/1.1", URL_BOOK, id);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Host: %s:%d", HOST, PORT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Authorization: Bearer %s\r\n", JWT);
    		compute_message(message, line);

    		free(line);

    		send_to_server(server_socket, message);
    		char *get_request_response = receive_from_server(server_socket);
    		printf("%s\n\n", get_request_response);

    		free(message);
    		}
    	}
    	else if(!strcmp(command, "add_book")) {
    		if(entered_library == 0)
    			printf("Trebuie sa ai acces la biblioteca!\n");
    		else {
    		//citire informatii despre carte
			char title[LINELEN];
    		char author[LINELEN];
    		char genre[LINELEN];
    		char publisher[LINELEN];
    		char page_count[LINELEN];

    		printf("title=");
    		fgets(title, LINELEN, stdin);
    		title[strlen(title)-1] = '\0';
    		printf("author=");
    		fgets(author, LINELEN, stdin);
    		author[strlen(author)-1] = '\0';
    		printf("genre=");
    		fgets(genre, LINELEN, stdin);
    		genre[strlen(genre)-1] = '\0';
    		printf("publisher=");
    		fgets(publisher, LINELEN, stdin);
    		publisher[strlen(publisher)-1] = '\0';
    		printf("page_count=");
    		fgets(page_count, LINELEN, stdin);
    		page_count[strlen(page_count)-1] = '\0';

    		server_socket = open_connection(ip, PORT, AF_INET, SOCK_STREAM, 0);

    		sprintf(line, "POST %s HTTP/1.1", URL_BOOKS);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Host: %s:%d", HOST, PORT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Authorization: Bearer %s", JWT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Content-Type: application/json");
    		compute_message(message, line);

    		int content_length = 66 + strlen(title) + strlen(author) +
    			strlen(genre) + strlen(publisher) + strlen(page_count);
    		memset(line, 0, LINELEN);
    		sprintf(line, "Content-Length: %d", content_length);
    		compute_message(message, line);

    		compute_message(message, "");

    		//adaugarea json-ului in mesaj (manual)
    		memset(line, 0, LINELEN);
    		char json[LINELEN];
    		strcpy(json, "{\"title\":\"%s\",\"author\":\"%s\",");
    		strcat(json, "\"genre\":\"%s\",\"page_count\":\"%s\",");
    		strcat(json, "\"publisher\":\"%s\"}");
    		sprintf(line, json, title, author, genre, page_count, publisher);
    		compute_message(message, line);

    		printf("%s\n", message);

    		free(line);

    		send_to_server(server_socket, message);
    		char *get_request_response = receive_from_server(server_socket);
    		printf("%s\n\n", get_request_response);

    		free(message);
    		}
    	}
    	else if(!strcmp(command, "delete_book")) {
    		if(entered_library == 0)
    			printf("Trebuie sa ai acces la biblioteca!\n");
    		else {
    		char id[LINELEN];

    		printf("id=");
    		fgets(id, LINELEN, stdin);
    		id[strlen(id)-1] = '\0';

    		server_socket = open_connection(ip, PORT, AF_INET, SOCK_STREAM, 0);

    		sprintf(line, "DELETE %s%s HTTP/1.1", URL_BOOK, id);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Host: %s:%d", HOST, PORT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Authorization: Bearer %s\r\n", JWT);
    		compute_message(message, line);

    		free(line);

    		send_to_server(server_socket, message);
    		char *get_request_response = receive_from_server(server_socket);
    		printf("%s\n\n", get_request_response);

    		free(message);
    		}
    	}
    	else if(!strcmp(command, "logout")) {
    		if(loggedin == 0)
    			printf("Trebuie sa fii logat!\n");
    		else {
    		server_socket = open_connection(ip, PORT, AF_INET, SOCK_STREAM, 0);

    		memset(line, 0, LINELEN);
    		sprintf(line, "GET %s HTTP/1.1", URL_LOGOUT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Host: %s:%d", HOST, PORT);
    		compute_message(message, line);

    		memset(line, 0, LINELEN);
    		sprintf(line, "Cookie: %s", cookie);
    		compute_message(message, line);

    		compute_message(message, "");

    		free(line);

    		send_to_server(server_socket, message);
    		char *get_request_response = receive_from_server(server_socket);
    		printf("%s\n", get_request_response);

    		free(message);

    		loggedin = 0;
    		entered_library = 0;
    		}
    	}
    	else if(!strcmp(command, "exit")) {
    		close_connection(server_socket);
    		break;
    	}
    	else {
    		printf("Comanda invalida!\n");
    	}
    }

    return 0;
}

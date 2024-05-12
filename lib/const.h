#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <sqlite3.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <openssl/evp.h>

char *VERSION = "1.1.0";

char *RED = "\033[1;31m"; 
char *GREEN = "\033[1;32m"; 
char *YELLOW = "\033[1;33m"; 
char *WHITE = "\033[1;37m"; 
char *PINK = "\033[1;35m"; 
char *BLUE = "\033[1;34m"; 
char *RESET = "\033[0m";

#define DATABASE_FILE "bookshop.db"
#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 100
#define MAX_GENRE_LENGTH 50
#define SHA256_DIGEST_LENGTH 32


int userRole;
char userName[50];

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
// Function to validate title
bool validateTitle(const char *title) {

    if (title[0] == '\0' || strlen(title) > MAX_TITLE_LENGTH) {
        printf("%sTitle was in wrong format. Please try again.\n%s", RED,RESET);
        return false;
    }
    return true;
}

// Function to validate author
bool validateAuthor(const char *author) {

    if (author[0] == '\0' || strlen(author) > MAX_AUTHOR_LENGTH) {
        printf("%sWrong input, please try again.\n%s",RED,RESET);
        return false;
    }
    return true;
}

// Function to validate genre
bool validateGenre(const char *genre) {

    if (genre[0] == '\0' || strlen(genre) > MAX_GENRE_LENGTH) {
        printf("%sWrong genre format. Please try again.\n%s",RED,RESET);
        return false;
    }
    return true;
}

// Function to validate price
bool validatePrice(float price) {
    if (price < 0) {
        printf("%sPrice must be non-negative. Please try again.\n%s",RED,RESET);
        return false;
    }
    return true;
}

// Function to validate quantity
bool validateQuantity(int quantity) {

    if (quantity < 0) {
        printf("%sQuantity must be non-negative. Please try again.\n%s");
        return false;
    }
    return true;
}

// Function to validate days
bool validateDays(int days) {

    if (days < 0) {
        printf("%sDays must be non-negative. Please try again.\n%s");
        return false;
    }
    return true;
}
// Function to validate ID
bool validateID(int id) {

    if (id < 0) {
        printf("%sDays must be non-negative. Please try again.\n%s");
        return false;
    }
    return true;
}
// Validate Username
bool validateUsername(const char *username) {
    // Username must not be empty and must have at least 4 characters
    if (strlen(username) <= 3   ) {
        printf("%sUsername must be at least 4 characters.\n%s",RED,RESET);
        return false;
    }
    return true;
}

bool validatePassword(const char *password) {
    // Password must not be empty and must have at least 5 characters
    if (strlen(password) <= 4) {
        printf("%sPassword is too short, Please try again.\n%s",RED,RESET);
        return false;
    }

    return true;
}

bool validateRole(const int role) {
    // Role should be either 0 or 1.
    return ( role >= 0 && role <=1);
}

bool validateEmail(const char *email) {
    // Basic email format validation (not comprehensive)
    int at_count = 0;
    int dot_count = 0;
    for (int i = 0; email[i] != '\0'; i++) {
        if (email[i] == '@') {
            at_count++;
        } else if (email[i] == '.') {
            dot_count++;
        }
    }
    return (at_count == 1 && dot_count > 0);
}


//**********************************************************************************************************************************
/*
  Intro
*/

void print_colored_line(const char *line, const char *foreground_color, const char *background_color) {
    printf("\033[%s;%sm%s\033[0m\n", foreground_color, background_color, line);
}

// Intro for the Bookery BMS.
void introBookery() {

    // Define color codes.
    const char *colors[] = {"34", "32", "33", "36", "35"};

    // ASCII art lines.
    const char *lines[] = {
    "██████   ██████   ██████  ██   ██ ███████ ██████  ██    ██",
    "██   ██ ██    ██ ██    ██ ██  ██  ██      ██   ██  ██  ██ ", 
    "██████  ██    ██ ██    ██ █████   █████   ██████    ████  ", 
    "██   ██ ██    ██ ██    ██ ██  ██  ██      ██   ██    ██    ",
    "██████   ██████   ██████  ██   ██ ███████ ██   ██    ██    ",  
    };

    // Print each line with color transition and a random character.
    printf("\n\n");

    for (int i = 0; i < 5; i++) {

        char line_copy[strlen(lines[i]) + 1];

        strcpy(line_copy, lines[i]);

        char rnd_char = 'O';

        for (int j = 0; line_copy[j] != '\0'; j++) {

            if (line_copy[j] == '@') {

                line_copy[j] = rnd_char;
            }
        }
        print_colored_line(line_copy, colors[i % 5], colors[(i + 1) % 5]);

        usleep(200000); // Sleep for 0.2 seconds.
    }
    printf("\n\n%sVersion: %s%s\n",PINK,VERSION,RESET);


    printf("\n\n");
}


void printMenu() {
    printf("\n*********** Bookshop Management System ***********\n");
    printf("1.  Add a Book\n");
    printf("2.  Display All Books\n");
    printf("3.  Search for a Book\n");
    printf("4.  Update Book Details\n");
    printf("5.  Sell a Book\n");
    printf("6.  Generate Sales Report\n");
    printf("7.  Rent a Book\n");
    printf("8.  Recall a Rent\n");
    printf("9.  Display all Rents\n");
    printf("10. Display late rent returns\n");
    printf("11. Search for a rent\n");
    printf("12. Generate Rental Report\n");
    printf("13. Advanced CLI\n");
    printf("0.  Exit\n");
    printf("******************************************************\n");
    printf("Enter your choice: ");
}

void help(const char *command) {

    if (strcmp(command, "del") == 0) {
        printf("Usage: del [user/book/allbooks]\n");
        printf("Description: Delete a user, a book or all the books.\n");

    } else if (strcmp(command, "add") == 0) {
        printf("Usage: add [user/book]\n");
        printf("Description: Add a new user or a new book.\n");

    } else if (strcmp(command, "login") == 0) {
        printf("Usage: login \n");
        printf("Description: Login to another account.\n");

    } else if (strcmp(command, "update") == 0) {
        printf("Usage: update [book/user] \n");
        printf("Description: Update the details of a book or a user.\n");
    }
    else if (strcmp(command, "show") == 0) {
        printf("Usage: show [users/books/rents]\n");
        printf("Description: Display all books, users or rent records.\n");

    }else if (strcmp(command, "search") == 0) {
        printf("Usage: search [book/rent]\n");
        printf("Description: Search for a book or a rent record.\n");

    }else if (strcmp(command, "sell") == 0) {
        printf("Usage: sell [book]\n");
        printf("Description: Sell a book.\n");
    }
    else if (strcmp(command, "rent") == 0) {
        printf("Usage: rent [book/recall/late]\n");
        printf("Description: Rent or recall a book\n");
    }
     else if(strcmp(command,"all") == 0){
        printf("%s*****Available commands******\n\n%s",BLUE,RESET);
        printf("1.    add user        -       Add a new user\n");
        printf("2.    add book        -       Add a new book\n");
        printf("3.    del user        -       Delete a user\n");
        printf("4.    del book        -       Delete a book\n");
        printf("5.    del allbooks    -       Delete all the books(%sno return%s).\n",RED,RESET);
        printf("6.    show books      -       Display all books\n");
        printf("7.    show users      -       Display all users\n");
        printf("8.    show rents      -       Display all rents\n");
        printf("9.    search book     -       Search for a book.\n");
        printf("10.   search rent     -       Search for a rent record\n");
        printf("11.   update book     -       Update the details of a book.\n");
        printf("12.   update user     -       Update the details of a user.\n"); 
        printf("13.   rent book       -       Rent a book\n");
        printf("14.   rent recall     -       Recall a rented book\n");
        printf("15.   rent late       -       Display Late rent returns\n");
        printf("16.   report sales    -       Generate sales report.\n"); 
        printf("17.   report rents    -       Generate sales report.\n"); 
        printf("18.   back            -       Go back to the previous menu\n");
        printf("19.   login           -       Login to another account.\n");
        printf("20.   help            -       Shows this help message\n");
        printf("21.   exit            -       Exit the program\n\n");
    } 
    else {
        printf("%sInvalid command:%s %s\n",RED,RESET,command);
    }
}
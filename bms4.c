#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <sqlite3.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <openssl/evp.h>

#include "lib/user.h"


// Function prototypes
int initializeDatabase();
void friendlyCLI();

int initializeDatabase() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    // Open database connection
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return return_code;
    }

    // SQL statement to create books table
    const char *sql_books = "CREATE TABLE IF NOT EXISTS books ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "title TEXT NOT NULL,"
                            "author TEXT NOT NULL,"
                            "genre TEXT,"
                            "price REAL,"
                            "quantity_available INTEGER,"   
                            "quantity_rented INTEGER,"
                            "quantity_sold INTEGER"
                            ");";

    // Execute SQL statement to create books table.
    return_code = sqlite3_exec(db, sql_books, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        // return return_code;
    }

    // SQL statement to create users table
    const char *sql_users = "CREATE TABLE IF NOT EXISTS users ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "username TEXT NOT NULL,"
                            "password TEXT NOT NULL,"
                            "email TEXT NOT NULL,"
                            "role INTEGER NOT NULL" 
                            ");";

    // Execute SQL statement to create users table
    return_code = sqlite3_exec(db, sql_users, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL Error: %s\n", errMsg);
        sqlite3_free(errMsg);
        // return return_code;
    }
    // SQL statement to create books table
    const char *sql_rents = "CREATE TABLE IF NOT EXISTS rents ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "title TEXT NOT NULL,"  
                            "Name TEXT NOT NULL,"
                            "Phone TEXT NOT NULL,"
                            "quantity_rented INTEGER,"
                            "rented_for_days INTEGER,"
                            "rent_date TEXT NOT NULL,"
                            "return_date TEXT"
                            ");";

    // Execute SQL statement to create books table.
    return_code = sqlite3_exec(db, sql_rents, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return return_code;
    }

    // Close database connection
    sqlite3_close(db);
    return 0;
}


//***********************************************************************************************************************************

/*
  Genrate raport for sales.
*/
void generateSalesReport() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n********** Sales Report **************\n");

    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented, quantity_sold FROM books;";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column
    int max_title_width = 0;
    int max_author_width = 0;
    int max_genre_width = 0;
    double max_price = 0.0;
    int max_qty_sold = 0;

    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, sqlite3_column_double(stmt, 3));
        max_qty_sold = fmax(max_qty_sold, sqlite3_column_int(stmt, 6));
    }

    printf("%s", BLUE);
    for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 45); i++) {
        printf("-");
    }
    printf("%s\n", RESET);

    // Print column headers
    printf("%s%-*s | %-*s | %-*s | %-6s | %-13s | %-7s | %s\n",
           BLUE,
           max_title_width, "Title",
           max_author_width, "Author",
           max_genre_width, "Genre",
           "Price",
           "Quantity Sold",
           "Revenue",
           RESET);

    printf("%s", BLUE);
    for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 45); i++) {
        printf("-");
    }
    printf("%s\n", RESET);

    // Print sales report data
    sqlite3_reset(stmt); // Reset the statement to re-execute
    float totalRevenue = 0;
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *title = (const char *)sqlite3_column_text(stmt, 0);
        const char *author = (const char *)sqlite3_column_text(stmt, 1);
        const char *genre = (const char *)sqlite3_column_text(stmt, 2);
        double price = sqlite3_column_double(stmt, 3);
        int quantitySold = sqlite3_column_int(stmt, 6);
        float revenue = price * quantitySold;
        printf("%-*s | %-*s | %-*s | $%-5.2f | %-13d | $%-6.2f\n",
               max_title_width, title,
               max_author_width, author,
               max_genre_width, genre,
               price,
               quantitySold,
               revenue);
        totalRevenue += revenue;
        for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 45); i++) {
            printf("-");
        }
        printf("\n");
    }

    printf("Total Revenue: $%.2f\n", totalRevenue);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


void advancedCLI() {
    printf("\033c");
    char command[100];
    
    do {
        printf("bms-> ");
        fgets(command, sizeof(command), stdin); 
        
        // Remove newline character if present.
        if (strlen(command) > 0 && command[strlen(command) - 1] == '\n') {
            command[strlen(command) - 1] = '\0';
        }
        
        if (strcmp(command, "add user") == 0) {
            // Call function to add user
            addUser();
        } else if (strcmp(command, "add book") == 0) {
            // Call function to add book
            addBook();
        } else if (strcmp(command, "login") == 0) {
            // Call function to login
            authenticateUser();
        } else if (strcmp(command, "update book") == 0) {
            // Call function to login
            updateBook();
        } else if (strcmp(command, "update user") == 0) {
            // Call function to login
            updateUser();   
        }
         else if (strcmp(command, "sell book") == 0) {
            // Call function to login
            sellBook();
        }  
        else if (strcmp(command, "del user") == 0) {
            // Call function to delete user
            delUser();
        } else if (strcmp(command, "del book") == 0) {
            // Call function to delete book
            delBook(1);
        }else if (strcmp(command, "del allbooks") == 0) {
            // Call function to delete all books
            delBook(0);
        }
        else if (strcmp(command, "rent book") == 0){
            // Call function to rent books
            rentBook();
        } else if (strcmp(command, "rent recall") == 0){
            // Call function to recall rented books 
            rentRecall();    
        }
        else if (strcmp(command, "show rents") == 0){
            // Call function to display rents
            displayRent();
        }else if (strcmp(command, "show books") == 0) {
            // Call function to display all books
            displayBooks();
        }else if (strcmp(command, "show users") == 0) {
            // Call function to display all users
            displayUsers();
        }
        else if (strcmp(command, "search rent") == 0){
            // Call function to search rented books
            searchRent();

        }else if (strcmp(command, "search book") == 0) {
            // Call function to display all users
            searchBook();
        }else if (strcmp(command, "report sales") == 0) {
            // Call function to display all users
            generateSalesReport();
        }else if (strcmp(command, "report rents") == 0) {
            // Call function to display all users
            searchBook();
        }
        else if (strcmp(command, "whoami") == 0) {
            whoami();
        }
         else if (strcmp(command, "help add") == 0) {
            help("add");
        } else if (strcmp(command, "help del") == 0) {
            help("del");
        } 
        else if (strcmp(command, "help show") == 0) {
            help("show");
        } else if (strcmp(command, "help login") == 0) {
            help("login");
        } else if (strcmp(command, "help search") == 0) {
            help("search");
        }else if (strcmp(command, "help update") == 0) {
            help("update");
        }else if (strcmp(command, "help sell") == 0) {
            help("sell");
        } else if (strcmp(command, "help rent") == 0) {
            help("rent");
        }
        else if (strcmp(command, "search") == 0) {
            help("search");
        }else if (strcmp(command, "update") == 0) {
            help("update");

        }else if (strcmp(command, "rent") == 0) {
            help("rent");

        }
        else if (strcmp(command, "del") == 0) {
            help("del");
        } else if (strcmp(command, "add") == 0) {
            help("add");
        } else if (strcmp(command, "show") == 0) {
            help("show");
        } else if (strcmp(command, "sell") == 0) {
            help("sell");
        }  
        else if (strcmp(command, "help") == 0) {
            help("all");
        }


         else if (strcmp(command, "back") == 0) {
            // Call function to go back
            friendlyCLI();
        }else if (strcmp(command, "clear") == 0) {
            // Call function to go back
            advancedCLI();
        } 
        else {
            if (strlen(command) > 0) {
                printf("%sInvalid command:%s %s\n",RED,RESET,command);
            }
        }
    } while (true);
}

void friendlyCLI(){
    printf("\033c");
    // Initialize database
    if (initializeDatabase() != 0) {
        fprintf(stderr, "Failed to initialize database.\n");
    }

    int choice;
    
    do {
        printMenu();
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                addBook();
                break;
            case 2:
                displayBooks();
                break;
            case 3:
                searchBook();
                break;
            case 4:
                updateBook();
                break;
            case 5:
                sellBook();
                break;
            case 6:
                generateSalesReport();
                break;
            case 7:
                rentBook();
                break;
            case 8:
                rentRecall();
                break;
            case 9:
                displayRent();
                break;
            case 10:
                searchRent();
                break;
            case 11:
                //Rent report
                break;
            case 12:
                advancedCLI();
            case 0:
                printf("Exiting program. Goodbye!\n");
                break;
            default:
                printf("%sInvalid choice.%s Please try again.\n",RED,RESET);
        }
        /*    printf("\n*********** Bookshop Management System ***********\n");
    printf("1. Add a Book\n");
    printf("2. Display All Books\n");
    printf("3. Search for a Book\n");
    printf("4. Update Book Details\n");
    printf("5. Sell a Book\n");
    printf("6. Generate Sales Report\n");
    printf("7. Rent a Book\n");
    printf("8. Recall a Rent\n");
    printf("9. Display all Rents\n");
    printf("10. Search for a rent\n");
    printf("11. Generate Rental Report\n");
    printf("12. Advanced CLI\n");
    printf("0. Exit\n");
    printf("******************************************************\n");
    printf("Enter your choice: ");*/
    } while(choice != 0);
    
}


int bms() {
    login();
    friendlyCLI();
}
int main(){
    bms();
    return 0;
}
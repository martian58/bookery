#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <sqlite3.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <openssl/evp.h>


#include "const.h"


// Define structure for a book.
struct Book {
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char genre[MAX_GENRE_LENGTH];
    float price;
    int quantity_available;
    int quantity_rented;
    int quantity_sold;
};

struct Rent {
    char title[MAX_TITLE_LENGTH];
    char customer_name[MAX_AUTHOR_LENGTH];
    char customer_phone[MAX_AUTHOR_LENGTH];
    int quantity_rented;
    int rented_for_days;
    char rented_date[MAX_AUTHOR_LENGTH];
    char return_date[MAX_AUTHOR_LENGTH];
};


//************************************************************************************************************************************************
/*
 Add Book.
*/

void addBook() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    struct Book newBook;
    do {
        printf("Enter title: ");
        scanf(" %[^\n]", newBook.title);
    } while (!validateTitle(newBook.title));

    // Input validation loop for author
    do {
        printf("Enter author: ");
        scanf(" %[^\n]", newBook.author);
    } while (!validateAuthor(newBook.author));

    // Input validation loop for genre
    do {
        printf("Enter genre: ");
        scanf(" %[^\n]", newBook.genre);
    } while (!validateGenre(newBook.genre));

    // Input validation loop for price
    do {
        printf("Enter price: ");
        scanf("%f", &newBook.price);
    } while (!validatePrice(newBook.price));

    // Input validation loop for quantity available
    do {
        printf("Enter quantity available: ");
        scanf("%d", &newBook.quantity_available);
    } while (!validateQuantity(newBook.quantity_available));
    newBook.quantity_rented = 0;
    newBook.quantity_sold = 0;

    char sql[1000];
    sprintf(sql, "INSERT INTO books (title, author, genre, price, quantity_available, quantity_rented, quantity_sold) VALUES ('%s', '%s', '%s', %.2f, %d, %d, %d);",
            newBook.title, newBook.author, newBook.genre, newBook.price, newBook.quantity_available, newBook.quantity_rented, newBook.quantity_sold);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("%sBook added successfully.\n%s",GREEN,RESET);
    }

    sqlite3_close(db);
}


//******************************************************************************************************************************************

void displayBooks() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n********** List of Books **************\n");

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
    int max_qty_available = 0;
    int max_qty_rented = 0;
    int max_qty_sold = 0;

    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, sqlite3_column_double(stmt, 3));
        max_qty_available = fmax(max_qty_available, sqlite3_column_int(stmt, 4));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 5));
        max_qty_sold = fmax(max_qty_sold, sqlite3_column_int(stmt, 6));
    }

    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print column headers
    printf("%s%-*s | %-*s | %-*s | %-6s | %-15s | %-18s | %s |%s\n",
           BLUE,
           max_title_width, "Title",
           max_author_width, "Author",
           max_genre_width, "Genre",
           "Price",
           "Quantity Available",
           "Quantity Rented",
           "Quantity Sold",
            RESET);


    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print book data
    sqlite3_reset(stmt); // Reset the statement to re-execute
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%-*s | %-*s | %-*s | $%-5.2f | %-18d | %-18d | %-13d |\n",
               max_title_width, (const char *)sqlite3_column_text(stmt, 0),
               max_author_width, (const char *)sqlite3_column_text(stmt, 1),
               max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
               sqlite3_column_double(stmt, 3),
               sqlite3_column_int(stmt, 4),
               sqlite3_column_int(stmt, 5),
               sqlite3_column_int(stmt, 6));
        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
             printf("-");
        }
        printf("\n");
        
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Search book

void searchBook() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char searchTerm[MAX_TITLE_LENGTH];
    printf("Enter search term (title, author, or genre): ");
    scanf(" %[^\n]s", searchTerm);

    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented, quantity_sold FROM books WHERE title LIKE ? OR author LIKE ? OR genre LIKE ?;";
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
    int max_qty_available = 0;
    int max_qty_rented = 0;
    int max_qty_sold = 0;

    // Bind search term to the prepared statement
    sqlite3_bind_text(stmt, 1, searchTerm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, searchTerm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, searchTerm, -1, SQLITE_STATIC);

    // Fetch data to calculate maximum widths
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, sqlite3_column_double(stmt, 3));
        max_qty_available = fmax(max_qty_available, sqlite3_column_int(stmt, 4));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 5));
        max_qty_sold = fmax(max_qty_sold, sqlite3_column_int(stmt, 6));
    }

    // Reset the statement to re-execute
    sqlite3_reset(stmt);

    printf("\n***** Search Results ******\n");
    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print column headers
    printf("%s%-*s | %-*s | %-*s | %-6s | %-15s | %-18s | %s | %s\n",
           BLUE,
           max_title_width, "Title",
           max_author_width, "Author",
           max_genre_width, "Genre",
           "Price",
           "Quantity Available",
           "Quantity Rented",
           "Quantity Sold",
            RESET);

    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);
    
    // Print search results with aligned columns    
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        if(strcmp((const char *)sqlite3_column_text(stmt,0),searchTerm) == 0){
            printf("%s%-*s %s| %-*s | %-*s | $%-5.2f | %-18d | %-18d | %-13d |\n",
                GREEN,max_title_width, (const char *)sqlite3_column_text(stmt, 0),RESET,
                max_author_width, (const char *)sqlite3_column_text(stmt, 1),
                max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6));
        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
            printf("-");
        }
        printf("\n");
        }
        else if(strcmp((const char *)sqlite3_column_text(stmt,1),searchTerm) == 0){
            printf("%-*s | %s%-*s %s| %-*s | $%-5.2f | %-18d | %-18d | %-13d |\n",
                max_title_width, (const char *)sqlite3_column_text(stmt, 0),
                GREEN,max_author_width, (const char *)sqlite3_column_text(stmt, 1),RESET,
                max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6));

        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
            printf("-");
        }
        printf("\n");
        }
        else if(strcmp((const char *)sqlite3_column_text(stmt,2),searchTerm) == 0){
            printf("%-*s | %-*s | %s%-*s %s| $%-5.2f | %-18d | %-18d | %-13d | \n",
                max_title_width, (const char *)sqlite3_column_text(stmt, 0),
                max_author_width, (const char *)sqlite3_column_text(stmt, 1),
                GREEN,max_genre_width, (const char *)sqlite3_column_text(stmt, 2),RESET,
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6));

        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
            printf("-");
        }
        printf("\n");
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
//*******************************************************************************************************************************************

/*
 Update book
*/

void updateBook() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char searchTitle[MAX_TITLE_LENGTH];
    do{
        printf("Enter the title of the book to update: ");
        scanf(" %[^\n]s", searchTitle);
    }while(!validateTitle(searchTitle));


    struct Book updatedBook;

    do{
        printf("Enter new title: ");
        scanf(" %[^\n]s", updatedBook.title);
    }while(!validateTitle(updatedBook.title));

    printf("Enter new author: ");
    scanf(" %[^\n]s", updatedBook.author);
    printf("Enter new genre: ");
    scanf(" %[^\n]s", updatedBook.genre);
    printf("Enter new price: ");
    scanf("%f", &updatedBook.price);
    printf("Enter new quantity available: ");
    scanf("%d", &updatedBook.quantity_available);

    char sql[1000];
    sprintf(sql, "UPDATE books SET title=?, author=?, genre=?, price=?, quantity_available=? WHERE title=?;");
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, updatedBook.title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, updatedBook.author, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, updatedBook.genre, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, updatedBook.price);
    sqlite3_bind_int(stmt, 5, updatedBook.quantity_available);
    sqlite3_bind_text(stmt, 6, searchTitle, -1, SQLITE_STATIC);

    return_code = sqlite3_step(stmt);
    if (return_code != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    } else {
        printf("%sBook details updated successfully.\n%s",GREEN,RESET);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

//***********************************************************************************************************************************************

// Sell Book

void sellBook() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char sellTitle[MAX_TITLE_LENGTH];
    do{
        printf("Enter the title of the book to sell: ");
        scanf(" %[^\n]s", sellTitle);
    }while(!validateTitle(sellTitle));
    
    int quantity;
    do{
        printf("Enter quantity to sell: ");
        scanf("%d", &quantity);
    }while(!validateQuantity(quantity));
    char sql[1000];
    sprintf(sql, "UPDATE books SET quantity_sold = quantity_sold + %d, quantity_available = quantity_available - %d WHERE title='%s';", quantity, quantity, sellTitle);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("%sSale successful.\n%s",GREEN,RESET);
    }

    sqlite3_close(db);
}

//************************************************************************************************************************************

// Delete a book.

void delBook(int mode){
    if(userRole != 0){
        printf("%sYou dont have permission for this action!\n this incident will be reported.\n%s",RED,RESET);
    }else{
        sqlite3 *db;
        char *errMsg = 0;
        int return_code;

        return_code = sqlite3_open(DATABASE_FILE, &db);
        if (return_code) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }
        if(mode == 1){
            
            char del_book[MAX_TITLE_LENGTH];
            do{
                printf("Enter the book to delete: ");
                scanf(" %[^\n]s", &del_book);
            }while(!validateUsername(del_book));


            char sql[1000];
            sprintf(sql, "DELETE FROM books WHERE title=?;");
            
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            sqlite3_bind_text(stmt, 1, del_book, -1, SQLITE_STATIC);


            return_code = sqlite3_step(stmt);
            if (return_code != SQLITE_DONE) {
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            } else {
                printf("%sBook deleted successfully.\n%s",GREEN,RESET);
            } 

            sqlite3_finalize(stmt);
            sqlite3_close(db);
        }
        else if(mode ==0){
            char* choice;
            printf("%sDelete all books(yes/no): %s",YELLOW,RESET);
            fgets(choice, sizeof(choice), stdin); 
            
            // Remove newline character if present.
            if (strlen(choice) > 0 && choice[strlen(choice) - 1] == '\n') {
                choice[strlen(choice) - 1] = '\0';
            }

            if(strcmp(choice,"yes") == 0){
                char sql[1000];
                sprintf(sql, "DELETE FROM books;");
                
                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

                return_code = sqlite3_step(stmt);
                if (return_code != SQLITE_DONE) {
                    fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
                } else {
                    printf("%sAll books deleted successfully.\n%s",GREEN,RESET);
                }
                sqlite3_finalize(stmt);
                sqlite3_close(db);
            }else{
                printf("%sDeletion aborted.\n%s",RED,RESET);
            }
 
        }
    
    }

}

//****************************************************************************************************************************************

/*
  Rent a book.

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
*/

void rentBook() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char rentTitle[MAX_TITLE_LENGTH];
    do{
        printf("Enter the title of the book to rent: ");
        scanf(" %[^\n]s", rentTitle);
    }while (!validateTitle(rentTitle));
    
    do{
        printf("Enter name of the customer: ");
        scanf
    }

    int days;
    printf("Enter number of days to rent: ");
    scanf("%d", &days);

    char sql[1000];
    char sql2[1000];
    sprintf(sql, "UPDATE books SET quantity_rented = quantity_rented + 1, quantity_available = quantity_available - 1 WHERE title='%s';", rentTitle);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("%sBook rented successfully for %d days.\n%s",GREEN, days,RESET);
    }

    sqlite3_close(db);
}
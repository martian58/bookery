#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <sqlite3.h>
#include <math.h>

#define DATABASE_FILE "bookshop.db"
#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 100
#define MAX_GENRE_LENGTH 100

// Define structure for a book
struct Book {
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char genre[MAX_GENRE_LENGTH];
    float price;
    int quantity_available;
    int quantity_rented;
};

// Function prototypes
int initializeDatabase();
void addBook();
void displayBooks();
void searchBook();
void updateBook();
void sellBook();
void generateSalesReport();
void rentBook();
void generateRentalReport();
void printMenu();



int initializeDatabase() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    // Open database connection
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
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
                            "quantity_rented INTEGER"
                            ");";

    // Execute SQL statement to create books table
    return_code = sqlite3_exec(db, sql_books, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return return_code;
    }

    // SQL statement to create users table
    const char *sql_users = "CREATE TABLE IF NOT EXISTS users ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "username TEXT NOT NULL,"
                            "password TEXT NOT NULL,"
                            "email TEXT NOT NULL"
                            ");";

    // Execute SQL statement to create users table
    return_code = sqlite3_exec(db, sql_users, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return return_code;
    }

    // Close database connection
    sqlite3_close(db);
    return 0;
}


void addBook() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    struct Book newBook;
    printf("Enter title: ");
    scanf(" %[^\n]s", newBook.title);
    printf("Enter author: ");
    scanf(" %[^\n]s", newBook.author);
    printf("Enter genre: ");
    scanf(" %[^\n]s", newBook.genre);
    printf("Enter price: ");
    scanf("%f", &newBook.price);
    printf("Enter quantity available: ");
    scanf("%d", &newBook.quantity_available);
    newBook.quantity_rented = 0;

    char sql[1000];
    sprintf(sql, "INSERT INTO books (title, author, genre, price, quantity_available, quantity_rented) VALUES ('%s', '%s', '%s', %.2f, %d, %d);",
            newBook.title, newBook.author, newBook.genre, newBook.price, newBook.quantity_available, newBook.quantity_rented);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Book added successfully.\n");
    }

    sqlite3_close(db);
}

void displayBooks() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n********** List of Books **************\n");

    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented FROM books;";
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

    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, sqlite3_column_double(stmt, 3));
        max_qty_available = fmax(max_qty_available, sqlite3_column_int(stmt, 4));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 5));
    }

    // Print column headers
    printf("%-*s | %-*s | %-*s | %-6s | %-20s | %s\n",
           max_title_width, "Title",
           max_author_width, "Author",
           max_genre_width, "Genre",
           "Price",
           "Quantity Available",
           "Quantity Rented");

    printf("-------------------------------------------------------------------------------------------------\n");

    // Print book data
    sqlite3_reset(stmt); // Reset the statement to re-execute
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%-*s | %-*s | %-*s | $%-5.2f | %-18d | %d\n",
               max_title_width, (const char *)sqlite3_column_text(stmt, 0),
               max_author_width, (const char *)sqlite3_column_text(stmt, 1),
               max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
               sqlite3_column_double(stmt, 3),
               sqlite3_column_int(stmt, 4),
               sqlite3_column_int(stmt, 5));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


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

    printf("\n===== Search Results =====\n");
    printf("Title\t\tAuthor\t\tGenre\t\tPrice\t\tQuantity Available\t\tQuantity Rented\n\n");
    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented FROM books WHERE title LIKE ? OR author LIKE ? OR genre LIKE ?;";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column
    int max_title_width = strlen("Title");
    int max_author_width = strlen("Author");
    int max_genre_width = strlen("Genre");
    double max_price = strlen("Price");
    int max_qty_available = strlen("Quantity Available");
    int max_qty_rented = strlen("Quantity Rented");

    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, strlen("$") + log10(sqlite3_column_double(stmt, 3)) + 3);
        max_qty_available = fmax(max_qty_available, (int)log10(sqlite3_column_int(stmt, 4)) + 1);
        max_qty_rented = fmax(max_qty_rented, (int)log10(sqlite3_column_int(stmt, 5)) + 1);
    }

    // Print search results with aligned columns
    sqlite3_reset(stmt);
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%-*s | %-*s | %-*s | $%*.2f | %*d | %*d\n",
               max_title_width, (const char *)sqlite3_column_text(stmt, 0),
               max_author_width, (const char *)sqlite3_column_text(stmt, 1),
               max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
               max_price, sqlite3_column_double(stmt, 3),
               max_qty_available, sqlite3_column_int(stmt, 4),
               max_qty_rented, sqlite3_column_int(stmt, 5));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


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
    printf("Enter the title of the book to update: ");
    scanf(" %[^\n]s", searchTitle);

    struct Book updatedBook;
    printf("Enter new title: ");
    scanf(" %[^\n]s", updatedBook.title);
    printf("Enter new author: ");
    scanf(" %[^\n]s", updatedBook.author);
    printf("Enter new genre: ");
    scanf(" %[^\n]s", updatedBook.genre);
    printf("Enter new price: ");
    scanf("%f", &updatedBook.price);
    printf("Enter new quantity available: ");
    scanf("%d", &updatedBook.quantity_available);

    char sql[1000];
    sprintf(sql, "UPDATE books SET title='%s', author='%s', genre='%s', price=%.2f, quantity_available=%d WHERE title='%s';",
            updatedBook.title, updatedBook.author, updatedBook.genre, updatedBook.price, updatedBook.quantity_available, searchTitle);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Book details updated successfully.\n");
    }

    sqlite3_close(db);
}

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
    printf("Enter the title of the book to sell: ");
    scanf(" %[^\n]s", sellTitle);

    int quantity;
    printf("Enter quantity to sell: ");
    scanf("%d", &quantity);

    char sql[1000];
    sprintf(sql, "UPDATE books SET quantity_available = quantity_available - %d WHERE title='%s';", quantity, sellTitle);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Sale successful.\n");
    }

    sqlite3_close(db);
}

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

    printf("\n===== Sales Report =====\n");
    printf("Title\tAuthor\tGenre\tPrice\tQuantity Sold\tRevenue\n");
    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented FROM books;";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    float totalRevenue = 0;
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        float price = sqlite3_column_double(stmt, 3);
        int quantitySold = sqlite3_column_int(stmt, 4) - sqlite3_column_int(stmt, 5);
        float revenue = price * quantitySold;
        printf("%s\t%s\t%s\t%.2f\t%d\t%.2f\n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2),
               price,
               quantitySold,
               revenue);
        totalRevenue += revenue;
    }
    printf("Total Revenue: %.2f\n", totalRevenue);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

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
    printf("Enter the title of the book to rent: ");
    scanf(" %[^\n]s", rentTitle);

    int days;
    printf("Enter number of days to rent: ");
    scanf("%d", &days);

    char sql[1000];
    sprintf(sql, "UPDATE books SET quantity_rented = quantity_rented + 1 WHERE title='%s';", rentTitle);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Book rented successfully for %d days.\n", days);
    }

    sqlite3_close(db);
}

void generateRentalReport() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n===== Rental Report =====\n");
    printf("Title\tPrice\tQuantity Rented\tRevenue\n");
    const char *sql = "SELECT title, price, quantity_rented FROM books;";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    float totalRevenue = 0;
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        float price = sqlite3_column_double(stmt, 1);
        int quantityRented = sqlite3_column_int(stmt, 2);
        float revenue = price * quantityRented;
        printf("%s\t%.2f\t%d\t%.2f\n",
               sqlite3_column_text(stmt, 0),
               price,
               quantityRented,
               revenue);
        totalRevenue += revenue;
    }
    printf("Total Revenue: %.2f\n", totalRevenue);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


bool authenticateUser() {
    char username[50], password[50];
    char sql[1000];
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);

    // Prompt the user to enter username and password
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    // Prepare SQL statement to query for user credentials
    sprintf(sql, "SELECT * FROM users WHERE username='%s' AND password='%s';", username, password);

    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return false;
    }

    // Execute the prepared statement
    return_code = sqlite3_step(stmt);
    if (return_code == SQLITE_ROW) {
        printf("Authentication successful. Welcome, %s!\n", username);
        sqlite3_finalize(stmt);
        return true;
    } else {
        printf("Authentication failed. Incorrect username or password.\n");
        sqlite3_finalize(stmt);
        return false;
    }
}


void printMenu() {
    printf("\n*********** Bookshop Management System ***********\n");
    printf("1. Add a Book\n");
    printf("2. Display All Books\n");
    printf("3. Search for a Book\n");
    printf("4. Update Book Details\n");
    printf("5. Sell a Book\n");
    printf("6. Generate Sales Report\n");
    printf("7. Rent a Book\n");
    printf("8. Generate Rental Report\n");
    printf("0. Exit\n");
    printf("******************************************************\n");
    printf("Enter your choice: ");
}


int main() {
    if(authenticateUser() != true){
        system("exit");
    }

    // Initialize database
    if (initializeDatabase() != 0) {
        fprintf(stderr, "Failed to initialize database.\n");
        return 1;
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
                generateRentalReport();
                break;
            case 0:
                printf("Exiting program. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while(choice != 0);
    
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <sqlite3.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <openssl/evp.h>

#include "book.h"

// Define structure for a user.
struct User {
    char username[50];
    char password[100];
    char email[100];
    int role; 
 
};

//*******************************************************************************************************************************************

/*
 Update user*/
void updateUser() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char searchUser[MAX_TITLE_LENGTH];
    do{
        printf("Enter the user to update: ");
        scanf(" %[^\n]s", searchUser);
    }while(!validateTitle(searchUser));


    struct User updatedUser;

    do{
        printf("Enter new username: ");
        scanf(" %[^\n]s", updatedUser.username);
    }while(!validateTitle(updatedUser.username));
    do{
        printf("Enter new email: ");
        scanf(" %[^\n]s", updatedUser.email);
    }while(!validateEmail(updatedUser.email));


        printf("Enter new role: ");
        scanf(" %[^\n]d", updatedUser.role);      
  


    char sql[1000];
    sprintf(sql, "UPDATE users SET username=?, email=?, role=? WHERE username=?;");
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, updatedUser.username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, updatedUser.email, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, updatedUser.role);
    sqlite3_bind_text(stmt, 4, searchUser, -1, SQLITE_STATIC);

    return_code = sqlite3_step(stmt);
    if (return_code != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    } else {
        printf("%sBook details updated successfully.%s\n",GREEN,RESET);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


//****************************************************************************************************************************************
/*
Add user.
*/

// Function to hash a password using SHA-256
void hashPassword(const char *password, unsigned char *hash) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned int md_len;

    md = EVP_sha256();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, password, strlen(password));
    EVP_DigestFinal_ex(mdctx, hash, &md_len);
    EVP_MD_CTX_free(mdctx);
}

// Who am i

void whoami(){
    if(userRole == 0){
        printf("%s : You are an admin user.\n",userName);
    }else{
        printf("%s : You are a user.\n",userName);
    }
}

//************************************************************************************************************************************

// Function to add users.

void addUser() {
    if(userRole != 0){
        printf("%sYou dont have permission for this action!\n this incident will be reported.\n%s",RED,RESET);
    }else{
        char *passwordPtr,*password2Ptr;
        char password[100], password2[100];
        sqlite3 *db;
        char *errMsg = 0;
        int return_code;

        return_code = sqlite3_open(DATABASE_FILE, &db);
        if (return_code) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        struct User newUser;
        do {
            printf("Enter username: ");
            scanf("%49s", newUser.username);
        } while (!validateUsername(newUser.username));

        do {
            passwordPtr = getpass("Enter password: ");
            strcpy(password, passwordPtr);
            password2Ptr = getpass("Enter password again: ");
            strcpy(password2, password2Ptr);
            if(strcmp(password,password2) != 0){
                printf("Passwords don't match\n");
            }else{
                strcpy(newUser.password, password);
            }

        } while (!validatePassword(newUser.password));

        do {
            printf("Enter email: ");
            scanf("%99s", newUser.email);
        } while (!validateEmail(newUser.email));

        do {
            printf("Enter role (0 for admin, 1 for regular user): ");
            scanf("%d", &newUser.role);
        } while (!validateRole(newUser.role));

        unsigned char hashed_password[SHA256_DIGEST_LENGTH];
        hashPassword(newUser.password, hashed_password);

        char hashed_password_str[SHA256_DIGEST_LENGTH * 2 + 1];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sprintf(&hashed_password_str[i * 2], "%02x", hashed_password[i]);
        }

        char sql[1000];
        sprintf(sql, "INSERT INTO users (username, password, email, role) VALUES (?, ?, ?, ?);");

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, newUser.username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashed_password_str, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, newUser.email, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, newUser.role);

        return_code = sqlite3_step(stmt);
        if (return_code != SQLITE_DONE) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        } else {
            printf("User added successfully.\n");
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

}

// Delete a user.

void delUser(){
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

        char del_username[MAX_AUTHOR_LENGTH];
        do{
            printf("Enter the user to delete: ");
            scanf(" %[^\n]s", &del_username);
        }while(!validateUsername(del_username));


        char sql[1000];
        sprintf(sql, "DELETE FROM users WHERE username=?;");
        
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, del_username, -1, SQLITE_STATIC);


        return_code = sqlite3_step(stmt);
        if (return_code != SQLITE_DONE) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        } else {
            printf("User deleted successfully.\n");
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

}


bool authenticateUser() {
    char username[50], *password;
    char sql[1000];
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);

    // Prompt the user to enter username and password
    printf("Enter username: ");
    scanf("%49s", username);
    password = getpass("Enter password: ");
    // scanf("%49s", password);

    unsigned char hashed_password[SHA256_DIGEST_LENGTH];
    hashPassword(password, hashed_password);

    // Convert the hashed password to a hexadecimal string
    char hashed_password_str[SHA256_DIGEST_LENGTH * 2 + 1]; // Add 1 for null terminator
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hashed_password_str[i * 2], "%02x", hashed_password[i]);
    }

    // Prepare SQL statement to query for user credentials and role
    sprintf(sql, "SELECT role FROM users WHERE username=? AND password=?;");

    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password_str, -1, SQLITE_STATIC);

    return_code = sqlite3_step(stmt);
    if (return_code == SQLITE_ROW) {
        int role = sqlite3_column_int(stmt, 0);
        printf("%sAuthentication successful!%s\n",GREEN ,RESET);
        strcpy(userName, username);
        userRole = role;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return true;
    } else {
        printf("%sIncorrect username or password.\n%s",RED,RESET);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
}

void login(){
    printf("\033c");
    introBookery();
    if(authenticateUser() != true){
        exit(0);
    }
}

//********************************************************************************************************************************************************

//Display users

void displayUsers() {
    if(userRole != 0){
        printf("%sYou dont have permission for this action!\n this incident will be reported.\n%s",RED,RESET);
    }else{
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int return_code;

        return_code = sqlite3_open(DATABASE_FILE, &db);
        if (return_code != SQLITE_OK) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        printf("\n********** List of Users **************\n");

        const char *sql = "SELECT username, email, role FROM users;";
        return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (return_code != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // Calculate maximum widths for each column
        int max_user_width = 0;
        int max_email_width = 0;
        int max_role_width = 5;
        

        while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
            max_user_width = fmax(max_user_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
            max_email_width = fmax(max_email_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
            
        }

        printf("%s",BLUE);
        for(int i =0;i < (max_user_width + max_email_width + max_role_width) + 8;i++){
            printf("-");
        }
        printf("%s\n",RESET);

        // Print column headers
        printf("%s%-*s | %-*s | %-*s |%s\n",
            BLUE,
            max_user_width, "User",
            max_email_width, "Email",
            max_role_width,"Role",
                RESET);


        printf("%s",BLUE);
        for(int i =0;i < (max_user_width + max_email_width + max_role_width) + 8;i++){
            printf("-");
        }
        printf("%s\n",RESET);

        // Print user data
        sqlite3_reset(stmt); // Reset the statement to re-execute
        while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
            printf("%-*s | %-*s | %-*s | \n",
                max_user_width, (const char *)sqlite3_column_text(stmt, 0),
                max_email_width, (const char *)sqlite3_column_text(stmt, 1),
                max_role_width, (const char *)sqlite3_column_text(stmt, 2));

            for(int i =0;i < (max_user_width + max_email_width + max_role_width) + 8;i++){
                printf("-");
            }
            printf("\n");
            
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);   
    }

}
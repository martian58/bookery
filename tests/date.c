#include <stdio.h>
#include <time.h>

int main() {
    char date_string[11]; // 10 characters for the date + 1 for the null terminator
    time_t t = time(NULL);
    struct tm *today = localtime(&t);
    
    strftime(date_string, sizeof(date_string), "%d/%m/%Y", today);
    
    printf("Today's date is: %s\n", date_string);
    
    return 0;
}


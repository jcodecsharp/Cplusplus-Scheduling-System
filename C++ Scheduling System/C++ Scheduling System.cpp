#include <ctime>
#include <iostream>
#include <chrono>
#include <regex>
#include <time.h>
#include <sqlite3.h> 

sqlite3* db;
sqlite3_stmt* stmt = NULL;
char* zErrMsg = 0;
int rc;
std::string sql;

int callback(void* NotUsed, int argc, char** argv, char** azColName) {

    for (int i = 0; i < argc; i++) {

        // Show column name, value, and newline
        std::cout << azColName[i] << ": " << argv[i] << std::endl;

    }

    
    return 0;
}

class Schedule {
private:
    void ViewSchedule();
    void CreateEvent();
    std::string GetTime();
    void InsertEvent(std::string eventDate, std::string eventTime,std::string eventDesc);
    void EditSchedule();
     
public:
    void CreateTable();
    void DisplayMenu();
};

void Schedule :: DisplayMenu() {


    // Gets the current date and time
    
    const std::chrono::time_point now{ std::chrono::system_clock::now() };
    const std::chrono::year_month_day ymd{ std::chrono::floor<std::chrono::days>(now) };
    std::cout << "Today's date: " << (ymd.month()) << "/" << (ymd.day()) << "/" << (ymd.year()) << "\n\n";

    // Select menu option on the schedule

    int scheduleInput;

    std::cout << "Welcome to your scheduler!\n\n";
    std::cout << "Press 1 to schedule a new event\n"
        "Press 2 to view your schedule\n"
        "Press 3 to edit your schedule\n\n"
        "Which option: ";

    while (true) {

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        
        std::cin >> scheduleInput;

        if (scheduleInput == 1) {
            CreateEvent();
            std::cout << "Press 1 to schedule a new event\n"
                "Press 2 to view your schedule\n"
                "Press 3 to edit your schedule\n\n"
                "Which option: ";
        }

        else if (scheduleInput == 2) {
            ViewSchedule();
            std::cout << "Press 1 to schedule a new event\n"
                "Press 2 to view your schedule\n"
                "Press 3 to edit your schedule\n\n"
                "Which option: ";
        }

        else if (scheduleInput == 3) {
            EditSchedule();
            std::cout << "Press 1 to schedule a new event\n"
                "Press 2 to view your schedule\n"
                "Press 3 to edit your schedule\n\n"
                "Which option: ";
        }

        else {
            std::cout << "Please enter one of the menu options.\n";

        }
    }
}


void Schedule::EditSchedule() {
   rc = sqlite3_open_v2("Scheduler.db", &db, 2, 0);

    std::cout << "\nThis is the edit menu option.\n"
        "Here, you can delete items off your schedule for the entire date or for a specific date and time.\n\n"
        "Enter the date you would like to delete from in format (mm/dd/yyyy) Example \"02/21/2021\" \n"
        "Enter date: ";
        
    std::string userChoice;
    std::string dateInput;
    std::string userTime;
    std::regex date("^(1[0-2]|0[1-9])/(3[01]|[12][0-9]|0[1-9])/[0-9]{4}$");
    while (true) {

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::cin >> dateInput;
        std::cout << "\n";
        
        if (std::regex_match(dateInput, date)) {

            sql = "Select * from Dates where Date = ?";
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
            sqlite3_bind_text(stmt, 1, dateInput.c_str(), -1, SQLITE_TRANSIENT);
            rc = sqlite3_step(stmt);

            if (rc != SQLITE_ROW) {

                std::cout << "Nothing was scheduled for that date. Back to the main menu.\n\n";
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                rc = sqlite3_finalize(stmt);
                break;

            }
            else {

                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                rc = sqlite3_finalize(stmt);

            }
            std::cout << "Enter \"Date\" to delete this entire day or \"time\" to delete a specific time\n"
                "from this day: ";

            label:
            std::cin >> userChoice;
            
            if (userChoice == "Date") {
                sql = "Delete from Dates WHERE Date = ?";
                sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
                sqlite3_bind_text(stmt, 1, dateInput.c_str(), -1, SQLITE_TRANSIENT);
                rc = sqlite3_step(stmt);
                std::cout << "Delete successful.\n";
                break;
            }
            else if (userChoice == "Time") {
    
                std::cout << "\nEnter the time you would like to delete from your schedule\n"
                    "in the format of HH/MM (AM/PM) Example: 09:30 AM.\n"
                    "Enter time: ";
                userTime = GetTime();
                sql = "Delete from Dates WHERE date = ? AND Time = ?";
                sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
                sqlite3_bind_text(stmt, 1, dateInput.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, userTime.c_str(), -1, SQLITE_TRANSIENT);
                rc = sqlite3_step(stmt);
                std::cout << "Delete Successful.";
               
            }
            else {
                std::cout << "Please enter \"Date\" or \"Time\": ";
                goto label;
            }
        }
        else {
            std::cout << "Incorrect date format. Please enter the date"
                "format of (mm/dd/yyyy) Example \"02/21/2021\": ";
        }
    }
    rc = sqlite3_close(db);
}



// Method to allow user to view their schedule for a specific date
void Schedule:: ViewSchedule() {
    rc = sqlite3_open_v2("Scheduler.db", &db, 2, 0);
    // Regular expressions for date input validation
    std::regex date("^(1[0-2]|0[1-9])/(3[01]|[12][0-9]|0[1-9])/[0-9]{4}$");
    std::string dateInput;
    
    
    // While loop to validate, search, and display data
    while (true) {
        std::cout << "\nEnter the month/day/year (mm/dd/yyyy) to view your schedule.\n"
            "(mm/dd/yyyy): ";

        // Clears input stream
        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        // Gets user input for the date
        std::cin >> dateInput;
       
        // If statement to check if the date matches a correct date format   
        if (std::regex_match(dateInput, date)) {
            sql = "Select time, event FROM Dates where DATE = ?";
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
            sqlite3_bind_text(stmt, 1, dateInput.c_str(), -1, SQLITE_TRANSIENT);
            
            // Checks if sql statement executes without any error / error codes
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Error selecting data: %s\n",
                    sqlite3_errmsg(db));
            }
            rc = sqlite3_step(stmt);
            int colNum = sqlite3_column_count(stmt);
            

            // Checks if the date entered by user contains any data
            if (rc == SQLITE_ROW) {
                std::cout << "\nYour Schedule for " << dateInput << "\n"
                    "---------------------------------\n";
                while (rc == SQLITE_ROW)
                {
                    
                    for (int i = 0; i < colNum; i++)
                    {
                        printf("%s:  ", sqlite3_column_text(stmt, i));
                    }
                    printf("\n");
                    rc = sqlite3_step(stmt);
                }
                std::cout << "---------------------------------\n";
            }

            // Executes if the date entered does not have any scheduled events / data
            else {
                std::cout << "Nothing found on the schedule for that date.\n\n";
                
            }
            break;
        }
        
        // Executes when user input an incorrect date format
        else {
            std::cout << "\nEnter a correct date in the format month/day/year (mm/dd/yyyy)\n"
                "Example: 07/21/2019: \n";
        }
    }

    // Resets the sql statement back to null and clears all bindings to the sql statement
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    rc = sqlite3_finalize(stmt);
    rc = sqlite3_close(db);
    std::cout << "\n";
}



// Method to allow user to Create and add events to their schedule
void Schedule::CreateEvent() {
    // Initialize the date string and set regular expression to match format mm/dd/yyyy
    std::string dateInput;
    std::string time;
    std::string eventDesc;
    std::regex date("^(1[0-2]|0[1-9])/(3[01]|[12][0-9]|0[1-9])/[0-9]{4}$");

    std::cout << "\nEnter the date you want to schedule an event for.\n" <<
     "Enter month/day/year (mm/dd/yyyy) to select the date.\n"
        "(mm/dd/yyyy): ";

    
    while (true) {

        // Clears the input stream if input stream contains error
        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        // Take's user date input as a string
        std::cin >> dateInput;

        // Validates user input for a date format
        if (std::regex_match(dateInput, date)) {
            rc = sqlite3_open_v2("Scheduler.db", &db,2,0);

            // Check if database connection succesful
            if (rc != 0)
            {
                fprintf(stderr, "Cannot open database: %s\n",   
                    sqlite3_errmsg(db));
                sqlite3_close(db);
                break;
            }

            // Gets the time and inserts the scheduled event
            std::cout << "\nEnter the time you would like to schedule this event for \n"
                "in the format of HH/MM (AM/PM) Example: 09:30 AM.\n"
                "Enter time: ";
                    time = GetTime();
                    std::cout << "\nYou set the time and date of event to: " << dateInput
                        << " " << time << ".\n" << "Enter the description of your event you're scheduling:\n";
                    std::getline(std::cin, eventDesc);
                    InsertEvent(dateInput, time, eventDesc);
                    sqlite3_clear_bindings(stmt);
                    sqlite3_reset(stmt);
                    rc = sqlite3_finalize(stmt);
                
                    std::cout << "\nEvent scheduled.\n\n";
            break;

        }

        // If the date format for the user input is wrong, request correct format
        else { std::cout << "Wrong format, enter the format request. (mm/dd/yyyy): \n"; }

    }
    rc = sqlite3_close(db);
}

// Helper method for Creating an event
void Schedule::InsertEvent(std::string eventDate, std::string eventTime, std::string eventDesc) {
    
    // SQL statement to insert event into table
    sql = "insert into Dates (DATE,TIME,EVENT) VALUES (?,?,?)";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, eventDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, eventTime.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, eventDesc.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);

}

// Helper method to return the correct time format
std::string Schedule::GetTime() {

    // Clears input stream
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string time;
    std::regex timeFormat("((1[0-2]|0[1-9]):([0-5][0-9]) ([AP][M])) *");

   

    // Gets the correct user input for the time then returns the time
    while (true) {

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::getline(std::cin, time);
        if (std::regex_match(time, timeFormat)) {
            break;
        }

        // Request correct format for the time
        else
        { 
            std::cout << "Regular expression did not match. \n"
                "Enter correct format of HH/MM (AM/PM) Example: 09:30 AM.\n"
                "Enter Time: "; 
        }
    }
    return time;
}


// Method to intially create the table when the program starts if it is not already created
void Schedule::CreateTable() {

    //Open database
    rc = sqlite3_open_v2("Scheduler.db", &db, 2, 0);



    // If database connection not succesful, print error.
    if (rc != 0)
    {
        fprintf(stderr, "Cannot open database: %s\n",
            sqlite3_errmsg(db));
        sqlite3_close(db);
    }



    //Check if table "Dates" already exists, if it does not exist, Create "Dates" table.

    sql = "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'Dates'";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

    if ((rc == SQLITE_OK) && (stmt != NULL)) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            sqlite3_reset(stmt);
            rc = sqlite3_finalize(stmt);
        }
        else {
            sql = "CREATE TABLE Dates("
                "DATE                  TEXT     NOT NULL,"
                "TIME                  TEXT     NOT NULL,"
                "EVENT                 TEXT     NOT NULL);";
            rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
            rc = sqlite3_step(stmt);
            if (rc != 0) {
                if (rc != 101) {
                    fprintf(stderr, "Error creating table: %s\n",
                        sqlite3_errmsg(db));
                    sqlite3_close(db);
                }
            }
            sqlite3_reset(stmt);
            rc = sqlite3_finalize(stmt);
        }
    }
    
    rc = sqlite3_close(db);
}



    int main()
    {
        Schedule schedule =  Schedule();
        schedule.CreateTable();
        schedule.DisplayMenu();
        system("pause");
    }


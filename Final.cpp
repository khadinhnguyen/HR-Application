//==============================================
// Name:           Kha Nguyen
// Student Number: xxxxxxx
// Email:          knguyen93@myseneca.ca
// Date:           Nov 30, 2020
//==============================================
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <occi.h>
#include <iomanip>
#include <cstring>

using oracle::occi::Environment;
using oracle::occi::Connection;

using namespace oracle::occi;
using namespace std;

struct Employee {
   int employeeNumber;
   char lastName[50];
   char firstName[50];
   char email[100];
   char phone[50];
   char extension[10];
   char reportsTo[100];
   char jobTitle[50];
   char city[50];
};

int menu(void);
int getInt(int min = 0, int max = 9999999999);
int findEmployee(Connection* conn, int employeeNumber, struct Employee* emp);
void displayEmployee(Connection* conn, struct Employee* emp);
void displayAllEmployee(Connection* conn);
void addEmployee(Connection* conn);
void insertEmployee(Connection* conn, struct Employee* insertEmp);
void updateEmployee(Connection* conn, int employeeNumber);
void deleteEmployee(Connection* conn, int employeeNumber);

int main(void) {
   // OCCI Variables
   Environment* env = nullptr;
   Connection* conn = nullptr;

   // User Variables
   string str;
   string usr = "dbs211_xxxxxx"; // for username
   string pass = "2161xxxxx"; // for password
   string srv = "myoracle12c.senecacollege.ca:1521/oracle12c";

    //Connect to server
   env = Environment::createEnvironment(Environment::DEFAULT);
   conn = env->createConnection(usr, pass, srv);

   bool keepGoing = true;
   int selection;

   do {
      cout << endl;
      selection = menu();
      cout << endl;
      try {
         switch (selection) {
         case 1:
            Employee requestEmployee;
            int searchingEmployee;
            int found;
            cout << "Please enter the employee number: ";
            cin >> searchingEmployee;
            found = findEmployee(conn, searchingEmployee, &requestEmployee);
            if (found == 0) cout << "Employee " << searchingEmployee << " does not exist" << endl;
            else displayEmployee(conn, &requestEmployee);
            break;
         case 2:
            displayAllEmployee(conn);
            break;
         case 3:
            addEmployee(conn);
            break;
         case 4:
            int updatingEmp;
            cout << "Employee Number: ";
            updatingEmp = getInt();
            updateEmployee(conn, updatingEmp);
            break;
         case 5:
            int deletingEmp;
            cout << "Employee Number: ";
            deletingEmp = getInt();
            deleteEmployee(conn, deletingEmp);
            break;
         case 6:
            keepGoing = false;
            break;
         default:
            break;
         }
         conn->commit();
      } catch (SQLException& sqlExcp) {
         cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
      }
   } while (keepGoing);

   env->terminateConnection(conn);
   Environment::terminateEnvironment(env);

   return 0;
}

int menu(void) {
   cout << "*********************HR Menu*********************" << endl;
   cout << "1) Find Employee" << endl << "2) Employees Report" << endl
      << "3) Add Employee" << endl << "4) Update Employee" << endl
      << "5) Remove Employee" << endl << "6) Exit" << endl << "> ";
   return getInt(1,6);
}

int getInt(int min, int max) {
   int value;
   bool badEntry;
   char nextChar;
   do {
      badEntry = false;
      cin >> value;
      if (cin.fail()) {
         cout << "Bad integer value, try again: ";
         cin.clear();
         cin.ignore(3000, '\n');
         badEntry = true;
      }
      else if (value < min || value > max) {
         cout << "Invalid value enterd, retry[" << min << " <= value <= "<< max << "]: ";
         cin.ignore(3000, '\n');
         badEntry = true;
      }
      else {
         nextChar = cin.get();
         if (nextChar != '\n') {
            cout << "Only enter an integer, try again: ";
            cin.ignore(3000, '\n');
            badEntry = true;
         }
      }
   } while (badEntry);
   return value;
}

int findEmployee(Connection* conn, int employeeNumber, struct Employee* emp) {
   int result = 0;
   bool found = false;
   Statement* stmt = conn->createStatement();
   ResultSet* rs = stmt->executeQuery("SELECT e.employeenumber, e.lastname, e.firstname, e.email, o.phone, e.extension, er.firstname || ' ' || er.lastname reportsTo, e.jobtitle, o.city FROM employees e LEFT OUTER JOIN offices o ON e.officecode = o.officecode LEFT OUTER JOIN employees er ON e.reportsto = er.employeenumber");

   while (rs->next() && !found) {
      emp->employeeNumber = rs->getInt(1);
      strcpy(emp->lastName, rs->getString(2).c_str());
      strcpy(emp->firstName, rs->getString(3).c_str());
      strcpy(emp->email, rs->getString(4).c_str());
      strcpy(emp->phone, rs->getString(5).c_str());
      strcpy(emp->extension, rs->getString(6).c_str());
      strcpy(emp->reportsTo, rs->getString(7).c_str());
      strcpy(emp->jobTitle, rs->getString(8).c_str());
      strcpy(emp->city, rs->getString(9).c_str());
      if (emp->employeeNumber == employeeNumber) {
         found = true;
         result = 1;
      }
   }
   conn->terminateStatement(stmt);
   return result;
}

void displayEmployee(Connection* conn, struct Employee* emp) {
   cout << "employeeNumber = " << emp->employeeNumber << endl;
   cout << "lastName = " << emp->lastName << endl;
   cout << "firstName = " << emp->firstName << endl;
   cout << "email = " << emp->email << endl;
   cout << "phone = " << emp->phone << endl;
   cout << "extension = " << emp->extension << endl;
   cout << "reportsTo = " << emp->reportsTo << endl;
   cout << "jobTitle = " << emp->jobTitle << endl;
   cout << "city = " << emp->city << endl;
}

void displayAllEmployee(Connection* conn) {
   int nameLenght = 0;
   int emailLength = 0;
   int managerLength = 0;

   Statement* stmt = conn->createStatement();
   ResultSet* rs = stmt->executeQuery("SELECT e.employeenumber, e.firstname || ' ' || e.lastname employeename, e.email, o.phone, e.extension, er.firstname || ' ' || er.lastname employeemanager FROM employees e LEFT OUTER JOIN offices o ON e.officecode = o.officecode LEFT OUTER JOIN employees er ON e.reportsto = er.employeenumber");
   cout << setw(8) << left << "E" << setw(20) << "Employee Name" << setw(40) << "Email" << setw(20) << "Phone" << setw(10) << "Ext" << setw(20) << "Manager" << endl;
   cout << setw(118) << setfill('-') << '-' << setfill(' ') << endl;
   if (!rs->next()) cout << "There is no employees's information to be display" << endl;
   while (rs->next()) {
      int empID = rs->getInt(1);
      string empName = rs->getString(2);
      if (empName.length() > nameLenght) nameLenght = empName.length();
      string empEmail = rs->getString(3);
      if (empEmail.length() > emailLength) emailLength = empEmail.length();
      string empPhone = rs->getString(4);
      string empEXT = rs->getString(5);
      string empManager = rs->getString(6);
      if (empManager.length() > managerLength) managerLength = empManager.length();
      cout << setw(8) << left << empID << setw(20) << empName << setw(40) << empEmail << setw(20) << empPhone << setw(10) << empEXT << setw(20) << empManager << endl;
   }
   conn->terminateStatement(stmt);

}

void insertEmployee(Connection* conn, struct Employee* insertEmp) {
   struct Employee existingEmp;
   int found = findEmployee(conn, insertEmp->employeeNumber, &existingEmp);
   if (found == 1) cout << "An employee with the same employee number exists" << endl;
   else {
      Statement* stmt = conn->createStatement();
      stmt->setSQL("INSERT INTO employees VALUES (:1, :2, :3, :4, :5, :6, :7, :8) ");
      stmt->setInt(1, insertEmp->employeeNumber);
      stmt->setString(2, insertEmp->lastName);
      stmt->setString(3, insertEmp->firstName);
      stmt->setString(4, insertEmp->extension);
      stmt->setString(5, insertEmp->email);
      stmt->setString(6, "1");
      stmt->setInt(7, 1002);
      stmt->setString(8, insertEmp->jobTitle);
      stmt->executeUpdate();
      cout << "The new employee is added successfully" << endl;
      conn->terminateStatement(stmt);
   }
}

void updateEmployee(Connection* conn, int employeeNumber) {
   struct Employee existingEmp;
   int found = findEmployee(conn, employeeNumber, &existingEmp);
   if (found == 0) cout << "The employee Number does not exist" << endl;
   else {
      cout << "Please enter the new extension: ";
      cin.get(existingEmp.extension, 10);
      cin.ignore(2000, '\n');
      Statement* stmt = conn->createStatement();
      stmt->setSQL("UPDATE employees SET extension = :1 WHERE employeenumber = :2");
      stmt->setString(1, existingEmp.extension);
      stmt->setInt(2, existingEmp.employeeNumber);
      stmt->executeUpdate();
      cout << "Update is successfully" << endl;
      conn->terminateStatement(stmt);
   }
  
}

void deleteEmployee(Connection* conn, int employeeNumber) {
   struct Employee existingEmp;
   int found = findEmployee(conn, employeeNumber, &existingEmp);
   if (found == 0) cout << "The employee does not exist" << endl;
   else {
      Statement* stmt = conn->createStatement();
      stmt->setSQL("DELETE FROM employees WHERE employeenumber = :1");
      stmt->setInt(1, existingEmp.employeeNumber);
      stmt->executeUpdate();
      cout << "Employee is deleted." << endl;
      conn->terminateStatement(stmt);
   }

}

void addEmployee(Connection* conn) {
   Employee addedEmployee;
   cout << "Please provide information as below: " << endl;
   cout << "Employee Number: ";
   addedEmployee.employeeNumber = getInt();
   cout << "Last Name: ";
   cin.get(addedEmployee.lastName, 50);
   cin.ignore(1000, '\n');
   cout << "First Name: ";
   cin.get(addedEmployee.firstName, 50);
   cin.ignore(1000, '\n');
   cout << "Email: ";
   cin.get(addedEmployee.email, 100);
   cin.ignore(1000, '\n');
   cout << "Extension: ";
   cin.get(addedEmployee.extension, 10);
   cin.ignore(1000, '\n');
   cout << "Job Title: ";
   cin.get(addedEmployee.jobTitle, 50);
   insertEmployee(conn, &addedEmployee);
}